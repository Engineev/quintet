#include "ServerIdentityCandidate.h"

#include <rpc/client.h>

quintet::ServerIdentityCandidate::ServerIdentityCandidate(quintet::ServerState &state_, quintet::ServerInfo &info_,
                                                          quintet::ServerService &service_)
        : ServerIdentityBase(state_, info_, service_) {}

std::vector<quintet::FutureWrapper<clmdep_msgpack::object_handle>> quintet::ServerIdentityCandidate::sendRequests() {
    std::vector<FutureWrapper<RPCLIB_MSGPACK::object_handle>> res;
    for (auto && srv : info.srvList) {
        if (srv == info.local)
            continue;
        service.logger("send RPCRequestVote to ", srv);


        res.emplace_back(service.rpcService.async_call(
                srv.addr, srv.port, "RequestVote",
                state.currentTerm, info.local,
                state.entries.size() - 1, state.entries.empty() ? InvalidTerm : state.entries.back().term
        ));
    }
    return res;
}

void quintet::ServerIdentityCandidate::launchVotesChecker(
        std::vector<quintet::FutureWrapper<RPCLIB_MSGPACK::object_handle>> &&votes) {
    for (auto && vote : votes) {
        // capture data by value !!
        vote.then([this, data = this->data](boost::future<RPCLIB_MSGPACK::object_handle> fut) mutable {
            Term termReceived;
            bool res;
            std::tie(termReceived, res) = fut.get().as<std::pair<Term, bool>>();

            service.logger("Vote result = ", res);

            // TODO: when larger term is received ...

            boost::lock_guard<boost::mutex> lk(data->m);
            if (data->discarded)
                return;
            data->votesReceived += res;
            // It is guaranteed that only one transformation will be carried out.
            if (data->votesReceived > info.srvList.size())
                service.identityTransformer.transform(ServerIdentityNo::Leader);
        });
    }
}

void quintet::ServerIdentityCandidate::leave() {
    boost::unique_lock<boost::mutex> lk(data->m);

    data->discarded = true;
    // Unlock before setting m to nullptr, otherwise it
    // may happen that this thread is the last thread
    // which hold m and set m to nullptr will destroy
    // the mutex before unlocking it.
    lk.unlock();
    data = nullptr;
}

void quintet::ServerIdentityCandidate::init() {
    ++state.currentTerm;
    state.votedFor = info.local;

    std::random_device rd;
    std::default_random_engine eg(rd());
    auto electionTimeout = info.electionTimeout + std::uniform_int_distribution<std::uint64_t>(0, info.electionTimeout)(eg);

    service.logger("Candidate::init()\n\telectionTimeout = ", electionTimeout,
        "\n\tTerm = ", state.currentTerm);

    service.heartBeatController.oneShot(
            [&] { service.identityTransformer.transform(ServerIdentityNo::Candidate); },
            electionTimeout);

    data = std::make_shared<ElectionData>();
//    launchVotesChecker(sendRequests());
    requestVotes();
}

std::pair<quintet::Term, bool>
quintet::ServerIdentityCandidate::RPCRequestVote(quintet::Term term, quintet::ServerId candidateId,
                                                 std::size_t lastLogIdx, quintet::Term lastLogTerm) {
    boost::upgrade_lock<boost::upgrade_mutex> lk(currentTermM);

    service.logger("\n\tIdentity = Candidate\n\tterm = ", term,
        ", from = ", candidateId, "\n\tcurrentTerm = ", state.currentTerm);

    if (term < state.currentTerm)
        return {state.currentTerm, false};
    if (term > state.currentTerm) {
        auto ulk = boost::upgrade_to_unique_lock<boost::upgrade_mutex>(lk);
        state.votedFor = NullServerId;
        state.currentTerm = term;
    }

    if ((state.votedFor == NullServerId || state.votedFor == candidateId)
        && upToDate(lastLogIdx, lastLogTerm)) {
        state.votedFor = candidateId;
        return {state.currentTerm, true};
    }
    return {state.currentTerm, false};
}

void quintet::ServerIdentityCandidate::requestVotes() {
    using Ms = boost::chrono::milliseconds;

    service.logger("Candidate::requestVotes");

    for (auto & srv : info.srvList) {
        if (srv == info.local)
            continue;
        boost::thread([this, data = this->data, &srv] () mutable {
            service.logger("send RPCRequestVote to ", srv);
            rpc::client c(srv.addr, srv.port);
            while (c.get_connection_state() != rpc::client::connection_state::connected) {
                service.logger("trying... (", srv, ")");
                boost::this_thread::sleep_for(Ms(1));
            }

            service.logger("connected! ", srv);

            Term termReceived;
            bool res;

            std::tie(termReceived, res) = c.call("RequestVote",
                                                 state.currentTerm, info.local,
                                                 state.entries.size() - 1, state.entries.empty() ? InvalidTerm : state.entries.back().term)
                    .as<std::pair<Term, bool>>();
            service.logger("Vote result from ", srv, " = ", res);

            // TODO: when larger term is received ...

            boost::lock_guard<boost::mutex> lk(data->m);
            if (data->discarded)
                return;
            data->votesReceived += res;
            // It is guaranteed that only one transformation will be carried out.
            if (data->votesReceived > info.srvList.size() / 2) {
                if (service.identityTransformer.transform(ServerIdentityNo::Leader)) {
#ifdef IDENTITY_TEST
                    for (auto & srv : info.srvList) {
                        if (srv == info.local)
                            continue;
                        rpc::client c(srv.addr, srv.port);
                        c.call("AppendEntries", 0, ServerId(), 0, 0, std::vector<LogEntry>(), 0);
                        service.logger("Shutdown ", srv);
                    }
#endif
                }
            }

        }).detach();
    }
}

