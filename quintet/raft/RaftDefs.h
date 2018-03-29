#ifndef QUINTET_RAFTDEFS_H
#define QUINTET_RAFTDEFS_H

#include <cstdint>
#include <tuple>

#include <rpc/server.h>

#include "Defs.h"

namespace quintet {

using Port  = std::uint16_t;
using Term  = std::uint64_t;
using Index = std::size_t;

struct LogEntry { // TODO
    Term        term;
    std::string opName;
    std::string args;
    PrmIdx      prmIdx;

    MSGPACK_DEFINE_ARRAY(term, opName, args, prmIdx);
};

enum class ServerIdentityNo {
    Follower = 0, Candidate, Leader, Down
};

} // namespace quintet

#endif //QUINTET_RAFTDEFS_H
