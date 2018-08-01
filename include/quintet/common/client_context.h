#ifndef QUINTET_CLIENT_CONTEXT_H
#define QUINTET_CLIENT_CONTEXT_H

#include <cstdint>

#include "common/macro.h"

namespace quintet {

class ClientContext {
public:
  GEN_HANDLES(timeout);

private:
  std::uint64_t timeout = std::uint64_t(-1);

}; // class ClientContext

} // namespace quintet

#endif //QUINTET_CLIENT_CONTEXT_H
