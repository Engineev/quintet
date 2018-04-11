#ifndef QUINTET_SERVERSTATE_H
#define QUINTET_SERVERSTATE_H

/**
 *  The states a server need to maintained in Raft.
 *  No other states should be included!
 *  See Figure 2 of the paper for reference
 */

#include <utility>
#include <vector>

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/lockable_adapter.hpp>

#include "raft/RaftDefs.h"
#include "raft/ServerInfo.h"

namespace quintet {

struct ServerState
        : public boost::shared_lockable_adapter<boost::shared_mutex> {

    Term currentTerm  = InvalidTerm;
    ServerId votedFor = NullServerId;
    std::vector<LogEntry> entries = {LogEntry()};
    // This add an empty log entry to the entries, in order to make the
    // entries's init size to be 1, and make it consistent of commitIdx with
    // the init size of entries - 1

    Index commitIdx   = 0;
    Index lastApplied = 0;

    std::vector<Index> nextIndex;
    std::vector<Index> matchIndex;
}; // struct ServerState

} // namespace quintet

namespace quintet {

bool upToDate(const ServerState & state, std::size_t lastLogIdx, Term lastLogTerm);

} // namespace quintet

#endif // QUINTET_SERVERSTATE_H
