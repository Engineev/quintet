#include "identity/IdentityFollower.h"
#include "identity/IdentityBaseImpl.h"

#include "misc/Rand.h"
#include "service/log/Common.h"
#include "misc/EventQueue.h"

/* -------------- constructors, destructors and Impl ------------------------ */

namespace quintet {

struct IdentityFollower::Impl : public IdentityBaseImpl {
  Impl(ServerState &state, const ServerInfo &info,
    ServerService &service, const RaftDebugContext & ctx)
    : IdentityBaseImpl(state, info, service, ctx) {
    service.logger.add_attribute(
      "Part", logging::attrs::constant<std::string>("Identity"));
  }

  void init();

  void leave();

  Reply appendEntries(const AppendEntriesMessage &msg);
}; // struct IdentityFollower::Impl

IdentityFollower::IdentityFollower(
  ServerState &state, const ServerInfo &info,
  ServerService &service, const RaftDebugContext & ctx)
    : pImpl(std::make_unique<Impl>(state, info, service, ctx)) {}

IdentityFollower::~IdentityFollower() = default;

} // namespace quintet

/* --------------- public member functions & RPC ---------------------------- */

namespace quintet {

void IdentityFollower::init() { pImpl->init(); }

void IdentityFollower::leave() { pImpl->leave(); }

Reply IdentityFollower::RPCAppendEntries(AppendEntriesMessage message) {
  return pImpl->appendEntries(message);
}

Reply IdentityFollower::RPCRequestVote(RequestVoteMessage message) {
  return pImpl->defaultRPCRequestVote(std::move(message));
}

AddLogReply IdentityFollower::RPCAddLog(AddLogMessage message) {
  return pImpl->defaultAddLog(std::move(message));
}

} // namespace quintet

/* -------------------- Helper functions ------------------------------------ */

namespace quintet {

void IdentityFollower::Impl::init() {
  auto electionTimeout =
    intRand(info.electionTimeout, info.electionTimeout * 2);
  service.heartBeatController.bind(electionTimeout, [this] {
    auto term = state.get_currentTerm();
    service.identityTransformer.notify(ServerIdentityNo::Candidate, term);
  });
  service.heartBeatController.start(false, false);
}

void IdentityFollower::Impl::leave() {
  service.heartBeatController.stop();
}

Reply IdentityFollower::Impl::appendEntries(const AppendEntriesMessage &msg) {
  {
    auto curTerm = state.get_currentTerm();
    if (msg.term < curTerm)
      return {curTerm, false};
  }
  service.heartBeatController.restart();
  return defaultRPCAppendEntries(msg);
}

} // namespace quintet

