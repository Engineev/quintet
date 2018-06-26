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
  int randId = intRand(100, 999);
  return pImpl->defaultRPCRequestVote(std::move(message),
                                      ServerIdentityNo::Follower, randId);
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
  BOOST_LOG(service.logger) << "electionTimeout = " << electionTimeout;
  service.heartBeatController.bind(electionTimeout, [this] {
    auto term = state.get_currentTerm();
    BOOST_LOG(service.logger) << "Time out! term = " << term;
    service.identityTransformer.notify(ServerIdentityNo::Candidate, term);
  });
  service.heartBeatController.start(false, false);
}

void IdentityFollower::Impl::leave() {
  service.heartBeatController.stop();
}

Reply IdentityFollower::Impl::appendEntries(const AppendEntriesMessage &msg) {
  int randId = intRand(1000, 9999);
  BOOST_LOG(service.logger)
    << "{" << randId << "} get RPCAppendEntries from "
    << msg.leaderId.toString();
  {
    auto curTerm = state.get_currentTerm();
    if (msg.term < curTerm) {
      BOOST_LOG(service.logger)
        << "{" << randId << "} msg.term = " << msg.term << " < "
        << curTerm << " = currentTerm. Return false";
      return {curTerm, false};
    }
  }
  service.heartBeatController.restart();
  return defaultRPCAppendEntries(msg, ServerIdentityNo::Follower, randId);
}

} // namespace quintet

