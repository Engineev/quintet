#include "ServerIdentityCandidate.h"

#include <boost/thread/lock_guard.hpp>
#include "Future.h"
#include "Utility.h"

#include <rpc/client.h>
#include <rpc/rpc_error.h>

quintet::ServerIdentityCandidate::ServerIdentityCandidate(quintet::ServerState &state_, quintet::ServerInfo &info_,
                                                          quintet::ServerService &service_)
        : ServerIdentityBase(state_, info_, service_) {}

void quintet::ServerIdentityCandidate::leave() {
    auto log = service.logger.makeLog("Candidate::leave");
    for (auto & t : requestingThreads)
        t.interrupt();
    for (auto & t : requestingThreads) {
        t.join();
        log.add("thread ", t.get_id(), " exits.");
    }
    requestingThreads.clear();
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

    votesReceived = 1;
    requestVotes(state.currentTerm, info.local,
                 state.entries.size() - 1, state.entries.empty() ? InvalidTerm : state.entries.back().term);
}

std::pair<quintet::Term, bool>
quintet::ServerIdentityCandidate::RPCRequestVote(quintet::Term term, quintet::ServerId candidateId,
                                                 std::size_t lastLogIdx, quintet::Term lastLogTerm) {
    boost::lock_guard<ServerState> lk(state);

    service.logger("RPCRequestVote: sleep!");
    service.faultInjector.randomSleep(0, info.electionTimeout);
    service.logger("RPCRequestVote: wake up!");

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

    service.logger("RPCAppendEntries: sleep!");
    service.faultInjector.randomSleep(0, info.electionTimeout);
    service.logger("RPCAppendEntries: wake up!");

    boost::lock_guard<ServerState> lk(state);
    if (term >= state.currentTerm) {
        state.currentTerm = term;
        service.identityTransformer.transform(ServerIdentityNo::Follower);
        return {state.currentTerm, false};
    }
    return {state.currentTerm, false};
}

std::pair<quintet::Term, bool>
quintet::ServerIdentityCandidate::sendRequestVote(quintet::ServerId target, quintet::Term currentTerm,
                                                  quintet::ServerId local, quintet::Index lastLogIdx,
                                                  quintet::Term lastLogTerm) {
    rpc::client c(target.addr, target.port);
    auto fut = toBoostFuture(c.async_call("RequestVote",
                                          currentTerm, local, lastLogIdx, lastLogTerm));
    if (fut.wait_for(boost::chrono::milliseconds(info.electionTimeout * 2)) != boost::future_status::ready) {
        return {0, false}; // TODO: return
    }

    return fut.get().as<std::pair<Term, bool>>();
}


void quintet::ServerIdentityCandidate::requestVotes(
        Term currentTerm, ServerId local, Index lastLogIdx, Term lastLogTerm) {
    service.logger("Candidate::requestVotes");

    for (auto & srv : info.srvList) {
        if (srv == info.local)
            continue;

        auto t = boost::thread([this, &srv,
                           currentTerm, local, lastLogIdx, lastLogTerm] () mutable {
            service.logger("send RPCRequestVote to ", srv);

            Term termReceived;
            bool res;
            try {
                std::tie(termReceived, res) = sendRequestVote(srv, currentTerm, local, lastLogIdx, lastLogTerm);
            } catch (boost::thread_interrupted & t) {
                service.logger(srv, " TLE");
                return;
            }
            service.logger("Vote result from ", srv, " = ", "(result: " , res, ", term: ", termReceived, ")");
            if (termReceived != currentTerm || !res)
                return;
            votesReceived += res;
            // It is guaranteed that only one transformation will be carried out.
            if (votesReceived > info.srvList.size() / 2) {
                if (service.identityTransformer.transform(ServerIdentityNo::Leader)) {
#ifdef IDENTITY_TEST
                    notifyReign(currentTerm);
#endif
                }
            }
        });

        requestingThreads.emplace_back(std::move(t));
    }
}





