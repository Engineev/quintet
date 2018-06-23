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
  std::cerr << "defaultAddLog unimplemented\n";
  return {false, NullServerId};
}

Reply IdentityBaseImpl::defaultRPCRequestVote(RequestVoteMessage msg) {
  checkRpcTerm(msg.term);
  auto defer = applyReadyEntries();
  // 1.
  if (msg.term < state.get_currentTerm()) {
    return {state.get_currentTerm(), false};
  }

  // 2.
  {
    boost::lock(state.currentTermM, state.voteForM);
    boost::lock_guard<boost::shared_mutex> currentTermLk(state.currentTermM,
                                                         boost::adopt_lock),
        voteForLk(state.voteForM, boost::adopt_lock);
    if (msg.term > state.currentTerm) {
      state.votedFor = msg.candidateId;
      state.currentTerm = msg.term;
      service.identityTransformer.notify(ServerIdentityNo::Follower, msg.term);
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
      state.votedFor = msg.candidateId;
      return {state.get_currentTerm(), true};
    }
  }
  return {state.get_currentTerm(), false};
}

Reply IdentityBaseImpl::defaultRPCAppendEntries(AppendEntriesMessage msg) {
  checkRpcTerm(msg.term);
  auto defer = applyReadyEntries();
  auto curTerm = state.get_currentTerm();
  { // 1.
    if (msg.term < curTerm)
      return {curTerm, false};
  }
  { // 2.
    boost::shared_lock<boost::shared_mutex> entriesLk(state.entriesM);
    if (state.entries.size() <= msg.prevLogIdx ||
        state.entries.at(msg.prevLogIdx).term != msg.term)
      return {curTerm, false};
  }
  { // 3. & 4.
    boost::lock_guard<boost::shared_mutex> entriesLk(state.entriesM);
    for (Index idxOffset = 0; idxOffset < msg.logEntries.size(); ++idxOffset) {
      Index entriesIdx = idxOffset + msg.prevLogIdx;
      if (state.entries.at(entriesIdx).term !=
          msg.logEntries.at(idxOffset).term) {
        state.entries.erase(state.entries.begin() + entriesIdx,
                            state.entries.end());
        state.entries.insert(state.entries.end(),
                             msg.logEntries.begin() + idxOffset,
                             msg.logEntries.end());
        break;
      }
    }
  }
  { // 5.
    boost::shared_lock<boost::shared_mutex> entriesLk(state.entriesM,
                                                      boost::defer_lock);
    boost::unique_lock<boost::shared_mutex> commitIdx(state.commitIdxM,
                                                      boost::defer_lock);
    if (msg.commitIdx > state.commitIdx) {
      assert(!state.entries.empty());
      state.commitIdx = std::min(msg.commitIdx, state.entries.size() - 1);
    }
  }
  return {curTerm, true};
}

bool IdentityBaseImpl::checkRpcTerm(Term term) {
  boost::lock_guard<boost::shared_mutex> lk(state.currentTermM);
  if (state.currentTerm > term) {
    state.currentTerm = term;
    service.identityTransformer.notify(ServerIdentityNo::Follower, term);
    return true;
  }
  return false;
}

std::shared_ptr<void> IdentityBaseImpl::applyReadyEntries() {
  boost::shared_lock<boost::shared_mutex> entriesLk(state.entriesM,
                                                    boost::defer_lock);
  boost::shared_lock<boost::shared_mutex> commitIdxLk(state.commitIdxM,
                                                      boost::defer_lock);
  boost::unique_lock<boost::shared_mutex> lastAppliedLk(state.lastAppliedM,
                                                        boost::defer_lock);
  boost::lock(entriesLk, commitIdxLk, lastAppliedLk);
  if (state.lastApplied >= state.commitIdx)
    return std::shared_ptr<void>();

  std::vector<LogEntry> entriesToApply(
      state.entries.begin() + state.lastApplied + 1,
      state.entries.begin() + state.commitIdx + 1);
  state.lastApplied = state.commitIdx;
  return std::shared_ptr<void>(
      nullptr,
      [this, entriesToApply = std::move(entriesToApply)](void *) mutable {
        service.apply(std::move(entriesToApply));
      });
}

} // namespace quintet