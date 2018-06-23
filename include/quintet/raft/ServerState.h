#pragma once

#include <vector>

#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/shared_mutex.hpp>

#include "RaftDefs.h"
#include "Macro.h"

namespace quintet {

struct ServerState
    : public boost::shared_lockable_adapter<boost::shared_mutex> {
  Term currentTerm = InvalidTerm;
  ServerId votedFor = NullServerId;
//  ServerId currentLeader = NullServerId;
  std::vector<LogEntry> entries = {LogEntry()};
  // This add an empty log entry to the entries, in order to make the
  // entries's init size to be 1, and make it consistent of commitIdx with
  // the init size of entries - 1

  Index commitIdx = 0;
  Index lastApplied = 0;

  mutable boost::shared_mutex
      currentTermM, voteForM, currentLeaderM, entriesM,
      commitIdxM, lastAppliedM;

  // some helper functions
  Term get_currentTerm() const;

  Index get_commitIdx() const;
};
// struct ServerState

} // namespace quintet