#ifndef QUINTET_RAFT_STATE_H
#define QUINTET_RAFT_STATE_H

#include "raft_common.h"

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/shared_lock_guard.hpp>

namespace quintet {

struct RaftState {
  Term currentTerm = InvalidTerm;
  ServerId votedFor;
//  ServerId currentLeader = NullServerId;
  std::vector<LogEntry> entries = { LogEntry() };

  Index commitIdx = 0;
  Index lastApplied = 0;

  ServerId curLeader;

  mutable boost::shared_mutex
      currentTermM, voteForM, currentLeaderM, entriesM,
      commitIdxM, lastAppliedM, curLeaderM;

  const Term syncGet_currentTerm() const {
    boost::shared_lock_guard<boost::shared_mutex> lk(currentTermM);
    return currentTerm;
  }

  const Index syncGet_commitIdx() const {
    boost::shared_lock_guard<boost::shared_mutex> lk(commitIdxM);
    return commitIdx;
  }
}; // class RaftState

} // namespace quintet

#endif //QUINTET_RAFT_STATE_H
