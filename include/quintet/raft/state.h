#ifndef QUINTET_STATE_H
#define QUINTET_STATE_H

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/shared_lock_guard.hpp>

#include "raft_common.h"
#include "common.h"

namespace quintet {
namespace raft {

struct State {
  State() = default;

  Term currentTerm = InvalidTerm;
  ServerId votedFor;
//  ServerId currentLeader = NullServerId;
  std::vector<LogEntry> entries = {LogEntry()};
  // This add an empty log entry to the entries, in order to make the
  // entries's init size to be 1, and make it consistent of commitIdx with
  // the init size of entries - 1

  Index commitIdx = 0;
  Index lastApplied = 0;

  ServerId curLeader;

  mutable boost::shared_mutex
      currentTermM, voteForM, currentLeaderM, entriesM,
      commitIdxM, lastAppliedM, curLeaderM;

  // some helper functions
  const Term syncGet_currentTerm() const {
    boost::shared_lock_guard<boost::shared_mutex> lk(currentTermM);
    return currentTerm;
  }

  const Index syncGet_commitIdx() const {
    boost::shared_lock_guard<boost::shared_mutex> lk(commitIdxM);
    return commitIdx;
  }

  const ServerId syncGet_curLeader() const {
    boost::shared_lock_guard<boost::shared_mutex> lk(curLeaderM);
    return curLeader;
  }
};

} // namespace raft
} // namespace quintet

#endif //QUINTET_STATE_H
