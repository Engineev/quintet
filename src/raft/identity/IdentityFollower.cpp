#include "identity/IdentityFollower.h"

#include "misc/Rand.h"
#include "service/log/Common.h"

/* -------------- constructors, destructors and Impl ------------------------ */

namespace quintet {

struct IdentityFollower::Impl : public IdentityBaseImpl {
  Impl(ServerState &state, ServerInfo &info, ServerService &service)
      : IdentityBaseImpl(state, info, service) {
    service.logger.add_attribute(
        "Part", logging::attrs::constant<std::string>("Identity"));
  }

  void init();

  void leave();

  Reply appendEntries(const AppendEntriesMessage &msg);

  Reply requestVote(const RequestVoteMessage &msg);

}; // struct IdentityFollower::Impl

IdentityFollower::IdentityFollower(ServerState &state, ServerInfo &info,
                                   ServerService &service)
    : pImpl(std::make_unique<Impl>(state, info, service)) {}

IdentityFollower::~IdentityFollower() = default;

} // namespace quintet

/* --------------- public member functions & RPC ---------------------------- */

namespace quintet {

void IdentityFollower::init() { pImpl->init(); }

void IdentityFollower::leave() { pImpl->leave(); }

Reply IdentityFollower::RPCAppendEntries(AppendEntriesMessage message) {
  return pImpl->appendEntries(message);
}

std::pair<Term /*current term*/, bool /*vote granted*/>
IdentityFollower::RPCRequestVote(RequestVoteMessage message) {
  return pImpl->requestVote(message);
}

AddLogReply IdentityFollower::RPCAddLog(AddLogMessage message) {
  return pImpl->defaultAddLog(std::move(message));
}

} // namespace quintet

/* -------------------- Helper functions ------------------------------------ */

namespace quintet {

void IdentityFollower::Impl::init() {
  ++state.getMutable_currentTerm();
  auto electionTimeout =
      intRand(info.electionTimeout, info.electionTimeout * 2);
  service.heartBeatController.bind(
      electionTimeout, [this, term = state.get_currentTerm()] {
        service.identityTransformer.notify(ServerIdentityNo::Candidate, term);
      });
  service.heartBeatController.start(false, false);
}

void IdentityFollower::Impl::leave() {
  service.heartBeatController.stop();
}

Reply IdentityFollower::Impl::appendEntries(const AppendEntriesMessage &msg) {
  boost::lock_guard<ServerState> lk(state);
  if (msg.term < state.get_currentTerm())
    return {state.get_currentTerm(), false};
  service.heartBeatController.restart();

  if (state.get_entries().size() <= msg.prevLogIdx ||
      state.get_entries().at(msg.prevLogIdx).term != msg.prevLogTerm) {
    return {state.get_currentTerm(), false};
  }

  for (Index idxOffset = 0; idxOffset < msg.logEntries.size(); ++idxOffset) {
    Index entriesIdx = idxOffset + msg.prevLogIdx;
    if (state.get_entries().at(entriesIdx).term !=
        msg.logEntries.at(idxOffset).term) {
      state.getMutable_entries().erase(state.get_entries().begin() + entriesIdx,
                                       state.get_entries().end());
      state.getMutable_entries().insert(state.get_entries().end(),
                                        msg.logEntries.begin() + idxOffset,
                                        msg.logEntries.end());
      break;
    }
  }

  if (msg.commitIdx > state.get_commitIdx()) {
    if (state.get_entries().empty()) {
      throw std::runtime_error("receive commit when own log is empty");
    }
    Index newCommitIdx = std::min(msg.commitIdx, state.get_entries().size() - 1);
    for (Index commitItem = state.get_commitIdx() + 1; commitItem <= newCommitIdx;
         ++commitItem) {
      service.apply(state.get_entries().at(commitItem));
    }
    state.getMutable_commitIdx() = newCommitIdx;
  }

  return {state.get_currentTerm(), true};
}

Reply IdentityFollower::Impl::requestVote(const RequestVoteMessage &msg) {
  boost::lock_guard<ServerState> lk(state);

  if (msg.term < state.get_currentTerm()) {
    return {state.get_currentTerm(), false};
  }
  if (msg.term > state.get_currentTerm()) {
    state.getMutable_votedFor() = NullServerId;
    state.getMutable_currentTerm() = msg.term;
  }

  // TODO: modify the persistent state at the end of the call
  if ((state.get_votedFor() == NullServerId ||
      state.get_votedFor() == msg.candidateId) &&
      upToDate(state, msg.lastLogIdx, msg.lastLogTerm)) {
    state.getMutable_votedFor() = msg.candidateId;
    return {state.get_currentTerm(), true};
  }

  return {state.get_currentTerm(), false};
}

} // namespace quintet