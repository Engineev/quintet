#ifndef QUINTET_RAFTDEFS_H
#define QUINTET_RAFTDEFS_H

#include <cstdint>
#include <cstddef>
#include <string>
#include <limits>
#include <functional>
//#include <iostream>  toString() is provided to avoid include iostream

#include "QuintetDefs.h"

namespace quintet {


using Term  = std::uint64_t;
using Index = std::size_t;

const Term InvalidTerm = 0;

const std::size_t IdentityNum = 5;
enum class ServerIdentityNo {
  Follower = 0, Candidate, Leader, Bogus, Down, Error,
};

const std::string IdentityNames[IdentityNum]
    = {"Follower", "Candidate", "Leader", "Bogus", "Down"};

enum class RpcStatus {
  OK, Error
};

struct RpcReply {
  RpcStatus rpcStatus;
  Term term;
  bool flag;
};

} /* namespace quintet */

namespace quintet {

struct LogEntry {
  Term term = InvalidTerm;
  std::string opName;
  std::string args;
  PrmIdx prmIdx;
  ServerId srvId;
};

} // namespace quintet

#endif //QUINTET_RAFTDEFS_H
