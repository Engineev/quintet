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

using Port  = std::uint16_t;
using Term  = std::uint64_t;
using Index = std::size_t;

const Term InvalidTerm = std::numeric_limits<Term>::max();

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

// ServerId
namespace quintet {

struct ServerId {
  std::string addr = "";
  Port port = 0;

  std::string toString() const;
};

const ServerId NullServerId{"", 0};

bool operator==(const ServerId &lhs, const ServerId &rhs);

bool operator!=(const ServerId & lhs, const ServerId & rhs);

} /* namespace quintet */

// hash
namespace std {
template <>
struct hash<quintet::ServerId> {
  std::size_t operator()(const quintet::ServerId& id) const {
    return std::hash<std::string>()(id.toString());
  }
};
} // namespace ::std

namespace quintet {

struct LogEntry {
  Term term;
  std::string opName;
  std::string args;
  PrmIdx prmIdx;
  ServerId srvId;
};

} // namespace quintet

#endif //QUINTET_RAFTDEFS_H
