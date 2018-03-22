#ifndef QUINTET_RAFTDEFS_H
#define QUINTET_RAFTDEFS_H

#include <cstdint>
#include <tuple>

namespace quintet {

using Port  = std::uint16_t;
using Term  = std::uint64_t;
using Index = std::size_t;

struct LogEntry {}; // TODO

} // namespace quintet

#endif //QUINTET_RAFTDEFS_H
