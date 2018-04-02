#include "ServerIdentityCandidate.h"

quintet::ServerIdentityCandidate::ServerIdentityCandidate(quintet::ServerState &state_, quintet::ServerInfo &info_,
                                                          quintet::ServerService &service_)
        : ServerIdentityBase(state_, info_, service_) {}

std::vector<quintet::FutureWrapper<clmdep_msgpack::object_handle>> quintet::ServerIdentityCandidate::sendRequests() {
    std::vector<FutureWrapper<RPCLIB_MSGPACK::object_handle>> res;
    for (auto && srv : info.srvList) {
        res.emplace_back(service.rpcService.async_call(
                srv.addr, srv.port, "RequestVote",
                state.currentTerm, info.local,
                state.entries.size() - 1, state.entries.back().term
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

    std::default_random_engine eg;
    service.heartBeatController.oneShot([&] { service.identityTransformer.transform(ServerIdentityNo::Candidate); },
        info.electionTimeout + std::uniform_int_distribution<std::uint64_t>(0, info.electionTimeout)(eg));

    data = std::make_shared<ElectionData>();
    launchVotesChecker(sendRequests());
}
