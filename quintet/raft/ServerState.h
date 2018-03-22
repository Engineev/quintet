#ifndef QUINTET_SERVERSTATE_H
#define QUINTET_SERVERSTATE_H

/**
 *  The states a server need to maintained in Raft.
 *  No other states should be included!
 *  See Figure 2 of the paper for reference
 */

#include <vector>

#include "quintet/raft/RaftDefs.h"
#include "quintet/raft/ServerInfo.h"

namespace quintet {

// TODO: thread-safe ??

class ServerState {
public:


private:
    Term     currentTerm;
    ServerId votedFor;
    // TODO: log[]

    Index commitIndex;
    Index lastApplied;

    std::vector<Index> nextIndex;
    std::vector<Index> matchIndex;

}; // class ServerState

} // namespace quintet


#endif //QUINTET_SERVERSTATE_H
