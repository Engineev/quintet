#ifndef QUINTET_RAFTDEFS_H
#define QUINTET_RAFTDEFS_H

#include <cstdint>
#include <tuple>
#include <limits>
#include <string>
#include <vector>

#include <rpc/server.h>

#include "Defs.h"

namespace quintet {

using Port  = std::uint16_t;
using Term  = std::uint64_t;
using Index = std::size_t;

const Term InvalidTerm = std::numeric_limits<Term>::max();

struct LogEntry { // TODO
    Term        term;
    std::string opName;
    std::string args;
    PrmIdx      prmIdx;

    MSGPACK_DEFINE_ARRAY(term, opName, args, prmIdx);
};

enum class ServerIdentityNo {
    Follower = 0, Candidate, Leader, Down, Error
};

const std::vector<std::string> IdentityNames = {
        "Follower", "Candidate", "Leader", "Down"
};

} // namespace quintet

#endif //QUINTET_RAFTDEFS_H
