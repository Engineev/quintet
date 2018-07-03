#ifndef QUINTET_CLIENTCONTEXT_H
#define QUINTET_CLIENTCONTEXT_H

#include <cstdint>

#include "misc/macro.h"

namespace quintet {

class ClientContext {
public:
  GEN_DEFAULT_CTOR_AND_ASSIGNMENT(ClientContext);

  void setTimeout(std::uint64_t timeout_) {
    timeout = timeout_;
  }

  const std::uint64_t getTimeout() const {
    return timeout;
  }

private:
  std::uint64_t timeout = std::uint64_t(-1);

}; // class Client Context

} // namespace quintet

#endif //QUINTET_CLIENTCONTEXT_H
