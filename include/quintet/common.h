#ifndef QUINTET_COMMON_H
#define QUINTET_COMMON_H

#include <cstdint>
#include <string>

#include "misc/macro.h"

namespace quintet {

using Port  = std::uint16_t;
using PrmIdx = std::uint64_t;

class ServerId {
public:
  GEN_DEFAULT_CTOR_AND_ASSIGNMENT(ServerId);
  ServerId(std::string addr, Port port);

  // e.g. 127.0.0.1:1080
  ServerId(const std::string & exAddr);

  GEN_CONST_HANDLE(addr);
  GEN_CONST_HANDLE(port);

  std::string toString(std::string sep = ":") const;

  const bool empty() const;

  void clear();

  bool operator==(const ServerId & rhs) const {
    return addr == rhs.addr && port == rhs.port;
  }

  bool operator!=(const ServerId & rhs) const {
    return !(*this == rhs);
  }

private:
  std::string addr;
  Port port = 0;
}; // class ServerId

class BasicLogEntry {
public:
  BasicLogEntry(std::string opName_, std::string args_, PrmIdx prmIdx)
      : opName(std::move(opName_)), args(std::move(args_)), prmIdx(prmIdx) {}

  GEN_DEFAULT_CTOR_AND_ASSIGNMENT(BasicLogEntry);
  GEN_CONST_HANDLE(opName);
  GEN_CONST_HANDLE(args);
  GEN_CONST_HANDLE(prmIdx);

private:
  std::string opName;
  std::string args;
  PrmIdx prmIdx = 0;
}; // class BasicLogEntry

} // namespace quintet

// hash
namespace std {
template <>
struct hash<quintet::ServerId> {
  std::size_t operator()(const quintet::ServerId& id) const {
    return std::hash<std::string>()(id.toString());
  }
};
} // namespace ::std

#endif //QUINTET_COMMON_H
