#include "ServerIdentityCandidate.h"

#include <memory>

#include <boost/thread/lock_guard.hpp>

#include <rpc/client.h>
#include <rpc/rpc_error.h>

#include "Future.h"
#include "Utility.h"
#include "log/Common.h"

#include <cstdlib>

quintet::ServerIdentityCandidate::ServerIdentityCandidate(quintet::ServerState &state_, quintet::ServerInfo &info_,
                                                          quintet::ServerService &service_)
        : ServerIdentityBase(state_, info_, service_) {}

void quintet::ServerIdentityCandidate::leave() {
    BOOST_LOG(service.logger) << "Candidate::leave()";
    service.heartBeatController.stop();
    for (auto & t : requestingThreads)
        t.interrupt();
    for (auto & t : requestingThreads)
        t.join();
    requestingThreads.clear();
}

void quintet::ServerIdentityCandidate::init() {
    BOOST_LOG(service.logger) << "Candidate::init()";
    ++state.currentTerm;
    state.votedFor = info.local;

    auto electionTimeout = Rand(info.electionTimeout, info.electionTimeout * 2)();

    BOOST_LOG(service.logger) << "Set electionTime = " << electionTimeout;

    votesReceived = 1;
    requestVotes(state.currentTerm, info.local,
                 state.entries.size() - 1, state.entries.empty() ? InvalidTerm : state.entries.back().term);

    service.heartBeatController.bind(
            [&, term = state.currentTerm] {
                BOOST_LOG(service.logger) << "Times out. Restart the election.";
                service.identityTransformer.notify(ServerIdentityNo::Candidate, term);
            },
            electionTimeout);
    service.heartBeatController.start(false, false);
}

std::pair<quintet::Term, bool>
quintet::ServerIdentityCandidate::RPCRequestVote(quintet::Term term, quintet::ServerId candidateId,
                                                 std::size_t lastLogIdx, quintet::Term lastLogTerm) {
    BOOST_LOG(service.logger) << "Candidate: Vote";
    boost::lock_guard<ServerState> lk(state);
    std::shared_ptr<void> defer(nullptr, [this](void*) {
        BOOST_LOG(service.logger) << "Candidate: voted";
    });

    if (term < state.currentTerm) {
        return {state.currentTerm, false};
    }
    if (term > state.currentTerm) {
        state.votedFor = NullServerId;
        state.currentTerm = term;
    }

    if ((state.votedFor == NullServerId || state.votedFor == candidateId)
        && upToDate(state, lastLogIdx, lastLogTerm)) {
        state.votedFor = candidateId;
        return {state.currentTerm, true};
    }

    return {state.currentTerm, false};
}

std::pair<quintet::Term, bool>
quintet::ServerIdentityCandidate::RPCAppendEntries(quintet::Term term, quintet::ServerId leaderId,
                                                   std::size_t prevLogIdx, quintet::Term prevLogTerm,
                                                   std::vector<quintet::LogEntry> logEntries, std::size_t commitIdx) {
    BOOST_LOG(service.logger) << "Candidate: Append";
    boost::lock_guard<ServerState> lk(state);
    std::shared_ptr<void> defer(nullptr, [this](void*) {
        BOOST_LOG(service.logger) << "Candidate: Appended";
    });
    if (term >= state.currentTerm) {
        state.currentTerm = term;
        service.identityTransformer.notify(ServerIdentityNo::Follower, state.currentTerm);
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
    for (auto & srv : info.srvList) {
        if (srv == info.local)
            continue;

        auto t = boost::thread([this, &srv,
                           currentTerm, local, lastLogIdx, lastLogTerm] () mutable {
            boost::this_thread::disable_interruption di;
            Term termReceived;
            bool res;
            try {
                boost::this_thread::restore_interruption ri(di);
                BOOST_LOG(service.logger) << "Send RPCRequestVote to " << srv.toString();
                std::tie(termReceived, res) = sendRequestVote(srv, currentTerm, local, lastLogIdx, lastLogTerm);
            } catch (boost::thread_interrupted & t) {
                return;
            }
            BOOST_LOG(service.logger) << "Receive the result of RPCRequestVote from " << srv.toString()
                                      << ". TermReceived = " << termReceived << ", res = " << res;
            if (termReceived != currentTerm || !res)
                return;
            votesReceived += res;
            // It is guaranteed that only one transformation will be carried out.
            if (votesReceived > info.srvList.size() / 2) {
                service.identityTransformer.notify(ServerIdentityNo::Leader, currentTerm);
            }
        });

        requestingThreads.emplace_back(std::move(t));
    }
}





