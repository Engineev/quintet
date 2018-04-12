#include "ServerIdentityCandidate.h"

#include <boost/thread/lock_guard.hpp>

#include <rpc/client.h>
#include <rpc/rpc_error.h>

quintet::ServerIdentityCandidate::ServerIdentityCandidate(quintet::ServerState &state_, quintet::ServerInfo &info_,
                                                          quintet::ServerService &service_)
        : ServerIdentityBase(state_, info_, service_) {}

void quintet::ServerIdentityCandidate::leave() {
    boost::unique_lock<boost::mutex> lk(data->m);

    data->discarded = true;
    // Unlock before setting m to nullptr, otherwise it
    // may happen that this thread is the last thread
    // which hold m and set data to nullptr will destroy
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
    requestVotes(state.currentTerm, info.local,
                 state.entries.size() - 1, state.entries.empty() ? InvalidTerm : state.entries.back().term);
}

std::pair<quintet::Term, bool>
quintet::ServerIdentityCandidate::RPCRequestVote(quintet::Term term, quintet::ServerId candidateId,
                                                 std::size_t lastLogIdx, quintet::Term lastLogTerm) {
    boost::lock_guard<ServerState> lk(state);

    auto log = service.logger.makeLog("RPCRequestVote");
    log.add("Identity = Candidate\n\tterm = ", term,
            ", from = ", candidateId, "\n\tcurrentTerm = ", state.currentTerm);

    if (term < state.currentTerm) {
        log.add("result = false. (term < currentTerm)");
        return {state.currentTerm, false};
    }
    if (term > state.currentTerm) {
        state.votedFor = NullServerId;
        state.currentTerm = term;
        log.add("term > currentTerm, voteFor <- null");
    }

    if ((state.votedFor == NullServerId || state.votedFor == candidateId)
        && upToDate(state, lastLogIdx, lastLogTerm)) {
        log.add("result = true");
        state.votedFor = candidateId;
        return {state.currentTerm, true};
    }

    if (state.votedFor != NullServerId && state.votedFor != candidateId)
        log.add("result = false (voted)");
    else
        log.add("result = false (not up-to-date)");
    return {state.currentTerm, false};
}

std::pair<quintet::Term, bool>
quintet::ServerIdentityCandidate::RPCAppendEntries(quintet::Term term, quintet::ServerId leaderId,
                                                   std::size_t prevLogIdx, quintet::Term prevLogTerm,
                                                   std::vector<quintet::LogEntry> logEntries, std::size_t commitIdx) {
    service.logger("Candidate:AppendEntries from ", leaderId);
    boost::lock_guard<ServerState> lk(state);
    if (term >= state.currentTerm) {
        state.currentTerm = term;
        service.identityTransformer.transform(ServerIdentityNo::Follower);
        return {state.currentTerm, false};
    }
    return {state.currentTerm, false};
}



void quintet::ServerIdentityCandidate::requestVotes(
        Term currentTerm, ServerId local, Index lastLogIdx, Term lastLogTerm) {
    service.logger("Candidate::requestVotes");

    for (auto & srv : info.srvList) {
        if (srv == info.local)
            continue;

        boost::thread([this, data = this->data, &srv,
                           currentTerm, local, lastLogIdx, lastLogTerm] () mutable {
            service.logger("send RPCRequestVote to ", srv);
            rpc::client c(srv.addr, srv.port);

            Term termReceived;
            bool res;
            try {
                // TODO: call RPCRequestVote
                std::tie(termReceived, res) = c.call("RequestVote",
                                                     currentTerm, local, lastLogIdx, lastLogTerm)
                        .as<std::pair<Term, bool>>();
            } catch (rpc::timeout & t) {
                service.logger(srv, " TLE");
                return;
            }
            service.logger("Vote result from ", srv, " = ", "(result: " , res, ", term: ", termReceived);
            if (termReceived != currentTerm || !res)
                return;

            boost::lock_guard<boost::mutex> lk(data->m);
            if (data->discarded)
                return;
            ++data->votesReceived;
            // It is guaranteed that only one transformation will be carried out.
            if (data->votesReceived > info.srvList.size() / 2) {
                if (service.identityTransformer.transform(ServerIdentityNo::Leader)) {
#ifdef IDENTITY_TEST
                    notifyReign(currentTerm);
#endif
                }
            }

        }).detach();
    }
}

