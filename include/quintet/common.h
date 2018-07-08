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

  GEN_CONST_HANDLE(addr);
  GEN_CONST_HANDLE(port);

  std::string toString(std::string sep = ":") const;

  explicit operator bool() const;

  void clear();

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

#endif //QUINTET_COMMON_H
