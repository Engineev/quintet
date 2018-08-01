#include "server/server.h"

#include "common/macro.h"
#include "server/config.h"
#include "server/raft.h"
#include "server/rpc_sender.h"
#include "server/rpc_receiver.h"

namespace quintet {

struct Server::Impl {
  /// Bind everything in the ctor
  Impl();

  Config config;
  Raft raft;
  RpcSender rpcSender;
  RpcReceiver rpcReceiver;



}; /* struct Server::Impl */

Server::Impl::Impl() {
  raft.bindMailboxes(config.getMailbox(), rpcSender.getMailbox());
  rpcReceiver.bindMailboxes(raft.getMailbox());
}

} /* namespace quintet */

namespace quintet {

GEN_PIMPL_CTOR(Server);
Server::~Server() = default;

} /* namespace quintet */