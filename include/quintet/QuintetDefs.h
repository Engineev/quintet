#ifndef QUINTET_QUINTETDEFS_H
#define QUINTET_QUINTETDEFS_H

#include <cstdint>
#include <string>

#include "Macro.h"

// ServerId
namespace quintet {

using PrmIdx = std::uint64_t;
using Port  = std::uint16_t;

struct ServerId {
  std::string addr = "";
  Port port = 0;

  std::string toString(const std::string & separator = ":") const {
    return addr + separator + std::to_string(port);
  }
};

const ServerId NullServerId{"", 0};

inline bool operator==(const ServerId &lhs, const ServerId &rhs) {
  return lhs.addr == rhs.addr && lhs.port == rhs.port;
}

inline bool operator!=(const ServerId &lhs, const ServerId &rhs) {
  return !(lhs == rhs);
}

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

class BasicLogEntry {
public:
  BasicLogEntry(std::string opName_, std::string args_,
               PrmIdx prmIdx, ServerId srvId_)
      : opName(std::move(opName_)), args(std::move(args_)),
        prmIdx(prmIdx), srvId(std::move(srvId_)) {}

  GEN_DEFAULT_CTOR_AND_ASSIGNMENT(BasicLogEntry);
  GEN_CONST_HANDLE(opName);
  GEN_CONST_HANDLE(args);
  GEN_CONST_HANDLE(prmIdx);
  GEN_CONST_HANDLE(srvId);

private:
  std::string opName;
  std::string args;
  PrmIdx prmIdx = 0;
  ServerId srvId;
}; // class BasicLogEntry

} // namespace quintet

#endif // QUINTET_QUINTETDEFS_H
