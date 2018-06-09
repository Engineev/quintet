#include "identity/IdentityCandidate.h"

#include <utility>

#include <boost/atomic.hpp>

#include "misc/Rand.h"

/* -------------- constructors, destructors and Impl ------------------------ */

namespace quintet {

IdentityCandidate::~IdentityCandidate() = default;

struct IdentityCandidate::Impl : public IdentityBase::IdentityBaseImpl {
  Impl(ServerState &state, ServerInfo &info, ServerService &service)
      : IdentityBaseImpl(state, info, service) {}

  boost::atomic<std::size_t> votesReceived;
  std::vector<boost::thread> requestingThreads;

  void init();

  void leave();

  std::pair<Term, bool> sendRequestVote(ServerId target, Term currentTerm,
                                        ServerId local, Index lastLogIdx,
                                        Term lastLogTerm) {
//    return service.clients.callRpcRequestVote(target, currentTerm, local, lastLogIdx,
//                                       lastLogTerm);
    // TODO: catch
  }
  void requestVotes();
};

IdentityCandidate::IdentityCandidate(ServerState &state, ServerInfo &info,
                                     ServerService &service)
    : pImpl(std::make_unique<Impl>(state, info, service)) {}

} // namespace quintet

/* ---------------- public member functions --------------------------------- */

namespace quintet {

void IdentityCandidate::init() { pImpl->init(); }

void IdentityCandidate::leave() { pImpl->leave(); }

} // namespace quintet

/* ---------------------- RPCs ---------------------------------------------- */

namespace quintet {

std::pair<Term /*current term*/, bool /*success*/>
IdentityCandidate::RPCAppendEntries(Term term, ServerId leaderId,
                                    std::size_t prevLogIdx, Term prevLogTerm,
                                    std::vector<LogEntry> logEntries,
                                    std::size_t commitIdx) {
  auto &service = pImpl->service;
  auto &state = pImpl->state;

  boost::lock_guard<ServerState> lk(state);
  if (term >= state.currentTerm) {
    state.currentTerm = term;
    service.identityTransformer.notify(ServerIdentityNo::Follower,
                                       state.currentTerm);
    return {state.currentTerm, false};
  }
  return {state.currentTerm, false};
}

std::pair<Term /*current term*/, bool /*vote granted*/>
IdentityCandidate::RPCRequestVote(Term term, ServerId candidateId,
                                  std::size_t lastLogIdx, Term lastLogTerm) {
  auto &state = pImpl->state;

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

  if ((state.votedFor == NullServerId || state.votedFor == candidateId) &&
      upToDate(state, lastLogIdx, lastLogTerm)) {
    state.votedFor = candidateId;
    return {state.currentTerm, true};
  }

  return {state.currentTerm, false};
}

} // namespace quintet

/* -------------------- Helper functions ------------------------------------ */

namespace quintet {

void IdentityCandidate::Impl::init() {
  ++state.currentTerm;
  state.votedFor = info.local;

  auto electionTimeout =
      intRand(info.electionTimeout, info.electionTimeout * 2);

  votesReceived = 1;
  requestVotes();

  service.heartBeatController.bind(
      electionTimeout, [this, term = state.currentTerm] {
        service.identityTransformer.notify(ServerIdentityNo::Candidate, term);
      });
  service.heartBeatController.start(false, false);
}

void IdentityCandidate::Impl::leave() {
  service.heartBeatController.stop();
  for (auto &t : requestingThreads)
    t.interrupt();
  for (auto &t : requestingThreads)
    t.join();
  requestingThreads.clear();
}

void IdentityCandidate::Impl::requestVotes() {
  for (auto &srv : info.srvList) {
    if (srv == info.local)
      continue;

    auto t = boost::thread([this, srv, currentTerm = state.currentTerm,
                            local = info.local,
                            lastLogIdx = state.entries.size() - 1,
                            lastLogTerm = state.entries.back().term]() mutable {
      boost::this_thread::disable_interruption di;
      Term termReceived;
      bool res;
      try {
        boost::this_thread::restore_interruption ri(di);
        std::tie(termReceived, res) =
            sendRequestVote(srv, currentTerm, local, lastLogIdx, lastLogTerm);
      } catch (boost::thread_interrupted &t) {
        return;
      }
      if (termReceived != InvalidTerm) {
      }
      if (termReceived != currentTerm || !res)
        return;
      votesReceived += res;
      // It is guaranteed that only one transformation will be carried out.
      if (votesReceived > info.srvList.size() / 2) {
        service.identityTransformer.notify(ServerIdentityNo::Leader,
                                           currentTerm);
      }
    });

    requestingThreads.emplace_back(std::move(t));
  }
}

} // namespace quintet