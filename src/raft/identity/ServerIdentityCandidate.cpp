#include "ServerIdentityCandidate.h"

#include <cassert>
#include <memory>
#include <random>
#include <chrono>

#include <boost/thread/lock_guard.hpp>
#include <boost/thread/thread.hpp>

#include "Future.h"
#include "Utility.h"
#include "ServerService.h"
#include "log/Common.h"

#include <cstdlib>

/* -------------- constructors, destructors and Impl ------------------------ */

namespace quintet {

ServerIdentityCandidate::~ServerIdentityCandidate() = default;

struct ServerIdentityCandidate::Impl : public IdentityBaseImpl {
    Impl(ServerState & state, ServerInfo & info, ServerService & service)
        : IdentityBaseImpl(state, info, service) {}

    std::atomic<std::size_t> votesReceived{0};
    std::vector<boost::thread> requestingThreads;

    /// \brief Send RPCRequestVotes to other servers and count the votes
    void requestVotes(Term currentTerm, ServerId local, Index lastLogIdx, Term lastLogTerm);

    std::pair<Term, bool> sendRequestVote(
        ServerId target, Term currentTerm, ServerId local,
        Index lastLogIdx, Term lastLogTerm);
};

ServerIdentityCandidate::ServerIdentityCandidate(
    ServerState &state, ServerInfo &info, ServerService &service)
    : pImpl(std::make_unique<Impl>(state, info, service))
{}

struct ServerIdentityCandidate::TestImpl {

};

} /* namespace quintet */

/* ---------------- public member functions --------------------------------- */

namespace quintet {

void ServerIdentityCandidate::init() {
    auto & service = pImpl->service;
    auto & state = pImpl->state;
    auto & info = pImpl->info;
    BOOST_LOG(service.logger)
        << "Candidate::init(). new term = " << state.currentTerm + 1;
    ++state.currentTerm;
    state.votedFor = info.local;

    auto electionTimeout = intRand(info.electionTimeout, info.electionTimeout * 2);

    BOOST_LOG(service.logger) << "Set electionTime = " << electionTimeout;

    pImpl->votesReceived = 1;
    pImpl->requestVotes(
        state.currentTerm, info.local, state.entries.size() - 1,
        state.entries.empty() ? InvalidTerm : state.entries.back().term);

    service.heartBeatController.bind(
        [&, term = state.currentTerm] {
            BOOST_LOG(service.logger) << "Times out. Restart the election.";
            service.identityTransformer.notify(ServerIdentityNo::Candidate, term);
        },
        electionTimeout);
    service.heartBeatController.start(false, false);
}

void ServerIdentityCandidate::leave() {
    auto & service = pImpl->service;
    BOOST_LOG(service.logger)
        << "Candidate::leave(), term = " << pImpl->state.currentTerm;
    service.heartBeatController.stop();
    for (auto & t : pImpl->requestingThreads)
        t.interrupt();
    BOOST_LOG(service.logger)
        << pImpl->requestingThreads.size() << " threads to join";
    for (auto & t : pImpl->requestingThreads) {
        auto start = std::chrono::high_resolution_clock::now();
        t.join();
        BOOST_LOG(service.logger)
            << "joint! Spent "
            << std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - start).count() << " ms";
    }
    pImpl->requestingThreads.clear();
}

} /* namespace quintet */

/* ---------------------- RPCs ---------------------------------------------- */

namespace quintet {

std::pair<Term, bool>
ServerIdentityCandidate::RPCRequestVote(
    Term term, ServerId candidateId, std::size_t lastLogIdx, Term lastLogTerm) {
    auto & state = pImpl->state;

    boost::lock_guard<ServerState> lk(state);

    if (term < state.currentTerm) {
        return {state.currentTerm, false};
    }
    if (term > state.currentTerm) {
        state.votedFor = NullServerId;
        state.currentTerm = term;
        pImpl->service.identityTransformer.notify(ServerIdentityNo::Follower, term);
//        return {state.currentTerm, false};
    }

    if ((state.votedFor == NullServerId || state.votedFor == candidateId)
        && upToDate(state, lastLogIdx, lastLogTerm)) {
        state.votedFor = candidateId;
        return {state.currentTerm, true};
    }

    return {state.currentTerm, false};
}

std::pair<Term, bool>
ServerIdentityCandidate::RPCAppendEntries(
    Term term, ServerId leaderId,
    std::size_t prevLogIdx, Term prevLogTerm,
    std::vector<LogEntry> logEntries, std::size_t commitIdx) {
    auto & service = pImpl->service;
    auto & state = pImpl->state;

    BOOST_LOG(service.logger) << "Candidate: Append";
    boost::lock_guard<ServerState> lk(state);
    std::shared_ptr<void> defer(nullptr, [&service](void*) {
        BOOST_LOG(service.logger) << "Candidate: Appended";
    });
    if (term >= state.currentTerm) {
        state.currentTerm = term;
        service.identityTransformer.notify(ServerIdentityNo::Follower, state.currentTerm);
        return {state.currentTerm, false};
    }
    return {state.currentTerm, false};
}


} /* namespace quintet */

/* -------------------- Impl functions -------------------------------------- */

namespace quintet {

std::pair<Term, bool>
ServerIdentityCandidate::Impl::sendRequestVote(
    ServerId target, Term currentTerm,
    ServerId local, Index lastLogIdx, Term lastLogTerm) {
    try {
        return service.rpcClients.call(
            target, "RequestVote", currentTerm, local, lastLogIdx, lastLogTerm
            ).as<std::pair<Term, bool>>();
    } catch (rpc::timeout & e) {
        BOOST_LOG(service.logger) << e.what();
        return {InvalidTerm, false};
    } catch (RpcClientNotExists & e) {
        BOOST_LOG(service.logger) << e.what();
        return {InvalidTerm, false};
    } catch (RpcNotConnected & e) {
        BOOST_LOG(service.logger) << "RpcNotConnected";
        return {InvalidTerm, false};
    }
}


void ServerIdentityCandidate::Impl::requestVotes(
    Term currentTerm, ServerId local, Index lastLogIdx, Term lastLogTerm) {
    for (auto & srv : info.srvList) {
        if (srv == info.local)
            continue;

        auto t = boost::thread([
            this, &srv, currentTerm, local, lastLogIdx, lastLogTerm] () mutable {
            boost::this_thread::disable_interruption di;
            Term termReceived;
            bool res;
            try {
                boost::this_thread::restore_interruption ri(di);
                BOOST_LOG(service.logger)
                    << "Send RPCRequestVote to " << srv.toString();
                std::tie(termReceived, res) =
                    sendRequestVote(srv, currentTerm, local, lastLogIdx, lastLogTerm);
            } catch (boost::thread_interrupted & t) {
                BOOST_LOG(service.logger) << "Be interrupted!";
                return;
            }
            if (termReceived != InvalidTerm) {
                BOOST_LOG(service.logger)
                    << "Receive the result of RPCRequestVote from " << srv.toString()
                    << ". TermReceived = " << termReceived << ", res = " << res;
            }
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

} /* namespace quintet */

/* --------------------- Test ----------------------------------------------- */

namespace quintet {

} /* namespace quintet */






