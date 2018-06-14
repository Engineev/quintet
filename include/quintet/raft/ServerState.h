#pragma once

#include <vector>

#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/shared_mutex.hpp>

#include "RaftDefs.h"

namespace quintet {

struct ServerState
    : public boost::shared_lockable_adapter<boost::shared_mutex> {

  Term currentTerm = InvalidTerm;
  ServerId votedFor = NullServerId;
  ServerId currentLeader = NullServerId;
  std::vector<LogEntry> entries = {LogEntry()};
  // This add an empty log entry to the entries, in order to make the
  // entries's init size to be 1, and make it consistent of commitIdx with
  // the init size of entries - 1

  Index commitIdx = 0;
  Index lastApplied = 0;

  std::vector<Index> nextIndex;
  std::vector<Index> matchIndex;
}; // struct ServerState

inline bool upToDate(const ServerState &state, std::size_t lastLogIdx,
              Term lastLogTerm) {
  if (state.entries.empty())
    return true;
  if (state.entries.back().term < lastLogTerm)
    return true;

  // candidate is up to date when
  // the two log lengths are equal
  if (state.entries.back().term == lastLogTerm &&
      state.entries.size() - 1 <= lastLogIdx)
    return true;

  return false;
}

} // namespace quintet