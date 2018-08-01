#include "server/server.h"

#include "common/macro.h"
#include "server/config.h"
#include "server/raft.h"
#include "server/rpc_sender.h"

namespace quintet {

struct Server::Impl {
  /// Bind everything in the ctor
  Impl();

  Config config;
  Raft raft;
  RpcSender rpcSender;


}; /* struct Server::Impl */

Server::Impl::Impl() {
  raft.bindMailboxes(config.getMailbox(), rpcSender.getMailbox());
}

} /* namespace quintet */

namespace quintet {

GEN_PIMPL_CTOR(Server);
Server::~Server() = default;

} /* namespace quintet */