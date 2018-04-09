#ifndef QUINTET_SERVERSTATE_H
#define QUINTET_SERVERSTATE_H

/**
 *  The states a server need to maintained in Raft.
 *  No other states should be included!
 *  See Figure 2 of the paper for reference
 */

#include <utility>
#include <vector>

#include "raft/RaftDefs.h"
#include "raft/ServerInfo.h"

namespace quintet {

struct ServerState {
    // update currentTerm at Server::init() ??
    Term currentTerm;
    ServerId votedFor = NullServerId;
    std::vector<LogEntry> entries;

    Index commitIndex;
    Index lastApplied;

    std::vector<Index> nextIndex;
    std::vector<Index> matchIndex;
}; // struct ServerState

} // namespace quintet

namespace quintet {

/* TODO: Why only non-member functions are provided?
 *
 */

/// \brief check whether the log provided is at least up-to-date
/// to the local logs.
///
/// \tparam Lock       The external lock on the entries.
/// \param entries     The local log entries.
/// \param lastLogIdx
/// \param lastLogTerm
/// \return whether the log is at least up-to-date
template <class Lock>
bool upToDate(std::pair<const std::vector<LogEntry> &, Lock &> entries,
              std::size_t lastLogIdx, Term lastLogTerm) {
    if (entries.first.empty()) return true;
    if (entries.first.back().term < lastLogTerm) return true;
    if (entries.first.back().term == lastLogTerm &&
        entries.first.size() - 1 <= lastLogIdx) // candidate is up to date when
                                                // the two log lengths are equal
        return true;
    return false;
}

} // namespace quintet

#endif // QUINTET_SERVERSTATE_H
