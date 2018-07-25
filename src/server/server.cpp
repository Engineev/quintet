#include "server/server.h"

#include "common/macro.h"
#include "server/config.h"
#include "server/raft.h"

namespace quintet {

struct Server::Impl {
  /// Bind everything
  Impl();

  Config config;
  Raft raft;


}; /* struct Server::Impl */

Server::Impl::Impl() {
  raft.bindMailboxes(config.getMailbox());
}

} /* namespace quintet */

namespace quintet {

GEN_PIMPL_CTOR(Server);
Server::~Server() = default;

} /* namespace quintet */