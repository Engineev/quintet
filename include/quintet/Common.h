#ifndef QUINTET_COMMON_H
#define QUINTET_COMMON_H

#include <cstdint>
#include <string>

#include "misc/macro.h"

namespace quintet {

using Port  = std::uint16_t;

class ServerId {
public:
  GEN_DEFAULT_CTOR_AND_ASSIGNMENT(ServerId);
  ServerId(std::string addr, Port port);

  GEN_CONST_HANDLE(addr);
  GEN_CONST_HANDLE(port);

  std::string toString(std::string sep = ":") const;

  operator bool() const;

private:
  std::string addr;
  Port port = 0;
}; // class ServerId

} // namespace quintet

#endif //QUINTET_COMMON_H
