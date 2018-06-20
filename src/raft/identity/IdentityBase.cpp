#include "identity/IdentityBase.h"

#include <boost/thread/lock_guard.hpp>

namespace quintet {

IdentityBase::IdentityBaseImpl::IdentityBaseImpl(
    ServerState &state, const ServerInfo &info,
    ServerService &service, const RaftDebugContext & debugContext)
    : state(state), info(info), service(service), debugContext(debugContext) {}

AddLogReply IdentityBase::IdentityBaseImpl::defaultAddLog(AddLogMessage) {
  boost::lock_guard<ServerState> lk(state);
  return {false, state.get_currentLeader()};
}

Reply IdentityBase::IdentityBaseImpl::defaultRPCRequestVote(RequestVoteMessage msg) {
  boost::lock_guard<ServerState> lk(state);

  if (msg.term < state.get_currentTerm()) {
    return {state.get_currentTerm(), false};
  }
  if (msg.term > state.get_currentTerm()) {
    state.getMutable_votedFor() = msg.candidateId;
    state.getMutable_currentTerm() = msg.term;
    service.identityTransformer.notify(ServerIdentityNo::Follower, msg.term);
    return {state.get_currentTerm(), true};
  }

  if ((state.get_votedFor() == NullServerId ||
      state.get_votedFor() == msg.candidateId) &&
      upToDate(state, msg.lastLogIdx, msg.lastLogTerm)) {
    state.getMutable_votedFor() = msg.candidateId;
    return {state.get_currentTerm(), true};
  }

  return {state.get_currentTerm(), false};
}

} // namespace quintet