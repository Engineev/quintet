#ifndef QUINTET_SERVERSTATE_H
#define QUINTET_SERVERSTATE_H

/**
 *  The states a server need to maintained in Raft.
 *  No other states should be included!
 *  See Figure 2 of the paper for reference
 */

#include <vector>

#include "raft/RaftDefs.h"
#include "raft/ServerInfo.h"

namespace quintet {

// TODO: thread-safe ??

struct ServerState {
    // update currentTerm at Server::init() ??
    Term currentTerm;
    ServerId votedFor = NullServerId;
    std::vector<LogEntry> entries;

    Index commitIdx;
    Index lastApplied;

    std::vector<Index> nextIndex;
    std::vector<Index> matchIndex;
    ServerState() {
        entries.push_back(LogEntry());
    } // This add an empty log entry to the entries, in order to make the
      // entries's init size to be 1, and make it consistent of commitIdx with
      // the init size of entries - 1

}; // struct ServerState

} // namespace quintet

#endif // QUINTET_SERVERSTATE_H
