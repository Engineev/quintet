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
    throw;
  }
  void requestVotes() { throw; }
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

namespace quintet {} // namespace quintet

/* -------------------- Helper functions ------------------------------------ */

namespace quintet {

void IdentityCandidate::Impl::init() {
  ++state.currentTerm;
  state.votedFor = info.local;

  auto electionTimeout =
      intRand(info.electionTimeout, info.electionTimeout * 2);

  votesReceived = 1;
  requestVotes();

  service.heartBeatController.bind(electionTimeout, [this,
                                                     term = state.currentTerm] {
    service.identityTransformer.notify(ServerIdentityNo::Candidate, term);
  });
  service.heartBeatController.start(false, false);
}

void IdentityCandidate::Impl::leave() {
  service.heartBeatController.stop();
  for (auto & t : requestingThreads)
    t.interrupt();
  for (auto & t : requestingThreads)
    t.join();
  requestingThreads.clear();
}

} // namespace quintet