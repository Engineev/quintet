#include "server/raft.h"

#include <memory>

namespace quintet {

struct Raft::Impl {
  std::unique_ptr<ConfigActor::Mailbox> toConfig;

}; /* struct Raft::Impl */

} /* namespace quintet */

namespace quintet {

GEN_PIMPL_CTOR(Raft);
GEN_PIMPL_DTOR(Raft);

} /* namespace quintet */