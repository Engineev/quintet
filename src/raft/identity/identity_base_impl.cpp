#include "raft/identity/identity_base_impl.h"

#include <boost/thread/locks.hpp>

namespace quintet {
namespace raft {

namespace {
bool upToDate(const std::vector<LogEntry> &entries, std::size_t lastLogIdx,
              Term lastLogTerm) {
  assert(!entries.empty());
  if (entries.back().get_term() < lastLogTerm) return true;
  return entries.back().get_term() == lastLogTerm &&
         entries.size() - 1 <= lastLogIdx;
}
} // namespace

IdentityBaseImpl::IdentityBaseImpl(State &state, const ServerInfo &info,
                                   Service &service,
                                   const DebugContext &debugContext)
    : state(state), info(info), service(service), debugContext(debugContext) {}

std::pair<bool, ServerId> IdentityBaseImpl::defaultAddLog(BasicLogEntry entry) {
  return {false, ServerId()};
}

Reply IdentityBaseImpl::defaultRPCRequestVote(RequestVoteMessage msg,
                                              IdentityNo identity, int randId) {
  using std::to_string;
  checkRpcTerm(msg.get_term(), identity);
  auto defer = applyReadyEntries();

  std::string logMsg = "false";
  auto defer2 =
      std::shared_ptr<void>(nullptr, [&logMsg, this, randId](void *) mutable {
        //    BOOST_LOG(service.logger) << "{" << randId << "} " << logMsg;
      });

  // 1.
  {
    boost::lock_guard<boost::shared_mutex> currentTermLk(state.currentTermM);
    if (msg.get_term() < state.currentTerm) {
      logMsg = "false: msg.term = " + to_string(msg.get_term()) + " < " +
               to_string(state.currentTerm) + " = curTerm";
      return {state.currentTerm, false};
    }
  }
  // 2.
  {
    boost::lock(state.currentTermM, state.voteForM);
    boost::lock_guard<boost::shared_mutex> currentTermLk(state.currentTermM,
                                                         boost::adopt_lock);
    boost::lock_guard<boost::shared_mutex> voteForLk(state.voteForM,
                                                     boost::adopt_lock);
    if (msg.get_term() > state.currentTerm) {
      logMsg = "true: msg.term = " + to_string(msg.get_term()) + " > " +
               to_string(state.currentTerm) + " = curTerm";
      state.votedFor = msg.get_candidateId();
      state.currentTerm = msg.get_term();
      if (identity != IdentityNo::Follower) {
        service.identityTransformer.notify(IdentityNo::Follower);
      } else {
        service.heartbeatController.restart();
      }
      return {state.currentTerm, true};
    }
  }

  {
    boost::unique_lock<boost::shared_mutex> voteForLk(state.voteForM,
                                                      boost::defer_lock);
    boost::shared_lock<boost::shared_mutex> entriesLk(state.entriesM,
                                                      boost::defer_lock);
    boost::shared_lock<boost::shared_mutex> currentTermLk(state.currentTermM,
                                                          boost::defer_lock);
    boost::lock(voteForLk, entriesLk, currentTermLk);
    if ((state.votedFor.empty() || state.votedFor == msg.get_candidateId()) &&
        upToDate(state.entries, msg.get_lastLogIdx(), msg.get_lastLogTerm())) {
      logMsg = "true: voteFor = " + state.votedFor.toString() + "; upToDate";
      state.votedFor = msg.get_candidateId();
      if (identity == IdentityNo::Follower)
        service.heartbeatController.restart();
      return {state.currentTerm, true};
    }
  }
  logMsg = "false: voteFor = " + state.votedFor.toString() + ";";
  return {state.currentTerm, false};
}

Reply IdentityBaseImpl::defaultRPCAppendEntries(AppendEntriesMessage msg,
                                                IdentityNo identity,
                                                int randId) {
  checkRpcTerm(msg.get_term(), identity);
  auto defer = applyReadyEntries();
  auto curTerm = state.syncGet_currentTerm();
  { // 1.
    if (msg.get_term() < curTerm) {
      //      BOOST_LOG(service.logger)
      //          << "{" << randId << "} msg.term = " << msg.term << " < "
      //          << curTerm << " = currentTerm. Return false";
      return {curTerm, false};
    }
  }
  { // 2. up to date ?
    boost::shared_lock<boost::shared_mutex> entriesLk(state.entriesM);
    //    bool upToDate = false;
    if (state.entries.size() <= msg.get_prevLogIdx() ||
        state.entries.at(msg.get_prevLogIdx()).get_term()
            != msg.get_prevLogTerm()) {
      //      if (state.entries.size() <= msg.get_prevLogIdx())
      //        BOOST_LOG(service.logger)
      //            << "{" << randId
      //            << "} entries.size() = "
      //            << state.entries.size() << " <= "
      //            << msg.prevLogIdx << " = prevLogIdx";
      //      else {
      //        BOOST_LOG(service.logger)
      //            << "{" << randId
      //            << "} local.prevLogTerm = " <<
      //            state.entries[msg.prevLogIdx].term
      //            << " != " << msg.prevLogTerm << " = msg.prevLogTerm";
      //      }
      return {curTerm, false};
    }
  }
  { // 3. & 4.
    boost::lock_guard<boost::shared_mutex> entriesLk(state.entriesM);
    Index idxOffset = 0;
    while (idxOffset < msg.get_logEntries().size() &&
           msg.get_prevLogIdx() + idxOffset + 1 < state.entries.size() &&
           state.entries[msg.get_prevLogIdx() + idxOffset + 1].get_term() ==
               msg.get_logEntries()[idxOffset].get_term())
      ++idxOffset;
    if (idxOffset != msg.get_logEntries().size()) { // Conflict does exist
      //      BOOST_LOG(service.logger)
      //          << "{" << randId << "} add " << msg.logEntries.size() -
      //          idxOffset
      //          << " entries.";
      state.entries.resize(msg.get_prevLogIdx() + idxOffset + 1);
      std::copy(msg.get_logEntries().cbegin() + idxOffset,
                msg.get_logEntries().cend(), std::back_inserter(state.entries));
    } else {
      //      BOOST_LOG(service.logger)
      //          << "{" << randId << "} Nothing to be appended";
    }
  }
  { // 5.
    boost::shared_lock<boost::shared_mutex> entriesLk(state.entriesM,
                                                      boost::defer_lock);
    boost::unique_lock<boost::shared_mutex> commitIdx(state.commitIdxM,
                                                      boost::defer_lock);
    if (msg.get_commitIdx() > state.commitIdx) {
      assert(!state.entries.empty());
      Index newCommitIdx =
          std::min(msg.get_commitIdx(), state.entries.size() - 1);
      if (state.commitIdx != newCommitIdx)
        //        BOOST_LOG(service.logger)
        //            << "commitIdx: " << state.commitIdx << " -> " <<
        //            newCommitIdx;
        state.commitIdx = newCommitIdx;
    }
  }
  //  BOOST_LOG(service.logger) << "{" << randId << "} succeed";
  return {curTerm, true};
}

bool IdentityBaseImpl::checkRpcTerm(Term term, IdentityNo identity) {
  boost::lock_guard<boost::shared_mutex> lk(state.currentTermM);
  if (state.currentTerm > term) {
    //    BOOST_LOG(service.logger)
    //        << "currentTerm: " << state.currentTerm << " -> " << term;
    state.currentTerm = term;
    if (identity != IdentityNo::Follower)
      service.identityTransformer.notify(IdentityNo::Follower);
    return true;
  }
  //  if (state.currentTerm == term && identity == ServerIdentityNo::Candidate)
  //  {
  //    service.identityTransformer.notify(ServerIdentityNo::Follower, term);
  //    return true;
  //  }
  return false;
}

std::shared_ptr<void> IdentityBaseImpl::applyReadyEntries() {
  return std::shared_ptr<void>(nullptr, [this](void *) mutable {
    boost::shared_lock<boost::shared_mutex> entriesLk(state.entriesM,
                                                      boost::defer_lock);
    boost::shared_lock<boost::shared_mutex> commitIdxLk(state.commitIdxM,
                                                        boost::defer_lock);
    boost::unique_lock<boost::shared_mutex> lastAppliedLk(state.lastAppliedM,
                                                          boost::defer_lock);
    boost::lock(entriesLk, commitIdxLk, lastAppliedLk);
    if (state.lastApplied >= state.commitIdx) return;

//    BOOST_LOG(service.logger)
//      << "apply " << state.lastApplied << "+1 ... " << state.commitIdx;
    std::vector<LogEntry> entriesToApply(
        state.entries.begin() + state.lastApplied + 1,
        state.entries.begin() + state.commitIdx + 1);
    state.lastApplied = state.commitIdx;
    service.apply(std::move(entriesToApply));
  });
}

} // namespace raft
} // namespace quintet