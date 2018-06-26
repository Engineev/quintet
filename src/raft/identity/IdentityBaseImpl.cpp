#include "identity/IdentityBaseImpl.h"

#include <boost/thread/locks.hpp>

namespace quintet {

namespace {
bool upToDate(const std::vector<LogEntry> &entries, std::size_t lastLogIdx,
              Term lastLogTerm) {
  assert(!entries.empty());
  if (entries.back().term < lastLogTerm)
    return true;
  return entries.back().term == lastLogTerm && entries.size() - 1 <= lastLogIdx;
}
} // namespace

IdentityBaseImpl::IdentityBaseImpl(ServerState &state, const ServerInfo &info,
                                   ServerService &service,
                                   const RaftDebugContext &debugContext)
    : state(state), info(info), service(service), debugContext(debugContext) {}

AddLogReply IdentityBaseImpl::defaultAddLog(AddLogMessage) {
//  std::cerr << "defaultAddLog unimplemented\n"; // TODO
  return {false, NullServerId};
}

Reply IdentityBaseImpl::defaultRPCRequestVote(RequestVoteMessage msg,
                                              ServerIdentityNo identity,
                                              int randId) {
  using std::to_string;
  checkRpcTerm(msg.term, identity);
  auto defer = applyReadyEntries();

  std::string logMsg = "false";
  auto defer2 = std::shared_ptr<void>(nullptr, [&logMsg, this, randId] (void *)
      mutable {
    BOOST_LOG(service.logger) << "{" << randId << "} " << logMsg;
  });

  // 1.
  if (msg.term < state.get_currentTerm()) {
    logMsg = "false: msg.term = " + to_string(msg.term) + " < " +
          to_string(state.get_currentTerm()) + " = curTerm";
    return {state.get_currentTerm(), false};
  }
  // 2.
  {
    boost::lock(state.currentTermM, state.voteForM);
    boost::lock_guard<boost::shared_mutex> currentTermLk(state.currentTermM,
                                                         boost::adopt_lock);
    boost::lock_guard<boost::shared_mutex> voteForLk(state.voteForM,
                                                     boost::adopt_lock);
    if (msg.term > state.currentTerm) {
      logMsg = "true: msg.term = " + to_string(msg.term) + " > " +
          to_string(state.currentTerm) + " = curTerm";
      state.votedFor = msg.candidateId;
      state.currentTerm = msg.term;
      if (identity != ServerIdentityNo::Follower) {
        service.identityTransformer.notify(ServerIdentityNo::Follower,
                                           msg.term);
      } else {
        service.heartBeatController.restart();
      }
      return {state.currentTerm, true};
    }
  }

  {
    boost::unique_lock<boost::shared_mutex> voteForLk(state.voteForM,
                                                      boost::defer_lock);
    boost::shared_lock<boost::shared_mutex> entriesLk(state.entriesM,
                                                      boost::defer_lock);
    boost::lock(voteForLk, entriesLk);
    if ((state.votedFor == NullServerId || state.votedFor == msg.candidateId) &&
        upToDate(state.entries, msg.lastLogIdx, msg.lastLogTerm)) {
      logMsg = "true: voteFor = " + state.votedFor.toString() + "; upToDate";
      state.votedFor = msg.candidateId;
      if (identity == ServerIdentityNo::Follower)
        service.heartBeatController.restart();
      return {state.get_currentTerm(), true};
    }
  }
  logMsg = "false: voteFor = " + state.votedFor.toString() + ";";
  return {state.get_currentTerm(), false};
}

Reply IdentityBaseImpl::defaultRPCAppendEntries(AppendEntriesMessage msg,
                                                ServerIdentityNo identity,
                                                int randId) {
  checkRpcTerm(msg.term, identity);
  auto defer = applyReadyEntries();
  auto curTerm = state.get_currentTerm();
  { // 1.
    if (msg.term < curTerm) {
      BOOST_LOG(service.logger)
        << "{" << randId << "} msg.term = " << msg.term << " < "
        << curTerm << " = currentTerm. Return false";
      return {curTerm, false};
    }
  }
  { // 2. up to date ?
    boost::shared_lock<boost::shared_mutex> entriesLk(state.entriesM);
//    bool upToDate = false;
    if (state.entries.size() <= msg.prevLogIdx ||
        state.entries.at(msg.prevLogIdx).term != msg.prevLogTerm) {
      if (state.entries.size() <= msg.prevLogIdx)
        BOOST_LOG(service.logger)
          << "{" << randId
          << "} entries.size() = "
          << state.entries.size() << " <= "
          << msg.prevLogIdx << " = prevLogIdx";
      else {
        BOOST_LOG(service.logger)
          << "{" << randId
          << "} local.prevLogTerm = " << state.entries[msg.prevLogIdx].term
          << " != " << msg.prevLogTerm << " = msg.prevLogTerm";
      }
      return {curTerm, false};
    }
  }
  { // 3. & 4.
    boost::lock_guard<boost::shared_mutex> entriesLk(state.entriesM);
    Index idxOffset = 0;
    while (idxOffset < msg.logEntries.size()
        && msg.prevLogIdx + idxOffset + 1 < state.entries.size()
        && state.entries[msg.prevLogIdx + idxOffset + 1].term
            == msg.logEntries[idxOffset].term)
      ++idxOffset;
    if (idxOffset != msg.logEntries.size()) { // Conflict does exist
      BOOST_LOG(service.logger)
        << "{" << randId << "} add " << msg.logEntries.size() - idxOffset
        << " entries.";
      state.entries.resize(msg.prevLogIdx + idxOffset + 1);
      std::copy(msg.logEntries.cbegin() + idxOffset, msg.logEntries.cend(),
                std::back_inserter(state.entries));
    } else {
      BOOST_LOG(service.logger)
        << "{" << randId << "} Nothing to be appended";
    }
  }
  { // 5.
    boost::shared_lock<boost::shared_mutex> entriesLk(state.entriesM,
                                                      boost::defer_lock);
    boost::unique_lock<boost::shared_mutex> commitIdx(state.commitIdxM,
                                                      boost::defer_lock);
    if (msg.commitIdx > state.commitIdx) {
      assert(!state.entries.empty());
      Index newCommitIdx = std::min(msg.commitIdx, state.entries.size() - 1);
      if (state.commitIdx != newCommitIdx)
        BOOST_LOG(service.logger)
          << "commitIdx: " << state.commitIdx << " -> " << newCommitIdx;
      state.commitIdx = newCommitIdx;
    }
  }
  BOOST_LOG(service.logger) << "{" << randId << "} succeed";
  return {curTerm, true};
}

bool IdentityBaseImpl::checkRpcTerm(Term term, ServerIdentityNo identity) {
  boost::lock_guard<boost::shared_mutex> lk(state.currentTermM);
  if (state.currentTerm > term) {
    state.currentTerm = term;
    if (identity != ServerIdentityNo::Follower)
      service.identityTransformer.notify(ServerIdentityNo::Follower, term);
    return true;
  }
//  if (state.currentTerm == term && identity == ServerIdentityNo::Candidate) {
//    service.identityTransformer.notify(ServerIdentityNo::Follower, term);
//    return true;
//  }
  return false;
}

std::shared_ptr<void> IdentityBaseImpl::applyReadyEntries() {
  return std::shared_ptr<void>(nullptr,
  [this](void *) mutable {
    boost::shared_lock<boost::shared_mutex> entriesLk(state.entriesM,
                                                      boost::defer_lock);
    boost::shared_lock<boost::shared_mutex> commitIdxLk(state.commitIdxM,
                                                        boost::defer_lock);
    boost::unique_lock<boost::shared_mutex> lastAppliedLk(state.lastAppliedM,
                                                          boost::defer_lock);
    boost::lock(entriesLk, commitIdxLk, lastAppliedLk);
    if (state.lastApplied >= state.commitIdx)
      return ;

    BOOST_LOG(service.logger)
      << "apply " << state.lastApplied << "+1 ... " << state.commitIdx;
    std::vector<LogEntry> entriesToApply(
        state.entries.begin() + state.lastApplied + 1,
        state.entries.begin() + state.commitIdx + 1);
    state.lastApplied = state.commitIdx;
    service.apply(std::move(entriesToApply));
  });
}

} // namespace quintet