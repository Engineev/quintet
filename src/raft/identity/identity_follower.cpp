#include "raft/identity/identity_follower.h"
#include "raft/identity/identity_base_impl.h"

#include "misc/rand.h"
//#include "raft/service/log/Common.h"
#include "misc/event_queue.h"

/* -------------- constructors, destructors and Impl ------------------------ */

namespace quintet {
namespace raft {

struct IdentityFollower::Impl : public IdentityBaseImpl {
  Impl(State &state, const ServerInfo &info, Service &service,
       const DebugContext &ctx)
      : IdentityBaseImpl(state, info, service, ctx) {
//    service.logger.add_attribute(
//        "Part", logging::attrs::constant<std::string>("Identity"));
  }
}; // struct IdentityFollower::Impl

IdentityFollower::IdentityFollower(
    State &state, const ServerInfo &info, Service &service,
    const DebugContext &ctx)
    : pImpl(std::make_unique<Impl>(state, info, service, ctx)) {}
IdentityFollower::~IdentityFollower() = default;

} // namespace raft
} // namespace quintet

/* -------------------------------------------------------------------------- */

namespace quintet {
namespace raft {

void IdentityFollower::init() {
  auto electionTimeout =
      intRand(pImpl->info.get_electionTimeout(),
              pImpl->info.get_electionTimeout() * 2);
//  BOOST_LOG(service.logger) << "electionTimeout = " << electionTimeout;
  pImpl->service.heartbeatController.bind(electionTimeout, [this] {
    auto term = pImpl->state.syncGet_currentTerm();
//    BOOST_LOG(service.logger) << "Time out! term = " << term;
    pImpl->service.identityTransformer.notify(IdentityNo::Candidate);
  });
  pImpl->service.heartbeatController.start(false, false);
}

void IdentityFollower::leave() {
  pImpl->service.heartbeatController.stop();
}

Reply IdentityFollower::RPCAppendEntries(AppendEntriesMessage msg, int rid) {
  int randId = intRand(1000, 9999);
//  BOOST_LOG(service.logger)
//      << "{" << randId << "} get RPCAppendEntries from "
//      << msg.leaderId.toString();
  {
    auto curTerm = pImpl->state.syncGet_currentTerm();
    if (msg.get_term() < curTerm) {
//      BOOST_LOG(service.logger)
//          << "{" << randId << "} msg.term = " << msg.term << " < "
//          << curTerm << " = currentTerm. Return false";
      return {curTerm, false};
    }
  }
  pImpl->service.heartbeatController.restart();
  return pImpl->defaultRPCAppendEntries(msg, IdentityNo::Follower, randId);
}

Reply IdentityFollower::RPCRequestVote(RequestVoteMessage message, int rid) {
  int randId = intRand(100, 999);
  return pImpl->defaultRPCRequestVote(std::move(message), IdentityNo::Follower,
                                      randId);
}

std::pair<bool, ServerId> IdentityFollower::AddLog(BasicLogEntry entry) {
  return pImpl->defaultAddLog(std::move(entry));
}

} // namespace raft
} // namespace quintet
