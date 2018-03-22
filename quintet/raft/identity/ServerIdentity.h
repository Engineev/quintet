#ifndef QUINTET_SERVERIDENTITY_H
#define QUINTET_SERVERIDENTITY_H

/// A universal header for identities

#include "quintet/raft/identity/ServerIdentityBase.h"

enum class ServerIdentityNo {
    Error = 0, Follower, Candidate, Leader
};

#endif //QUINTET_SERVERIDENTITY_H
