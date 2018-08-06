
#include <server/server.h>
#include <quintet/server/server.h>

#include "server/server.h"

#include "common/macro.h"
#include "server/config.h"
#include "server/identity_transformer.h"
#include "server/raft.h"
#include "server/rpc_sender.h"
#include "server/rpc_receiver.h"
#include "server/timer.h"
#include "server/logger.h"

namespace quintet {

struct Server::Impl {
  /// Bind everything in the ctor
  Impl();

  Config config;
  Raft raft;
  RpcSender rpcSender;
  RpcReceiver rpcReceiver;
  IdentityTransformer identityTransformer;
  Timer timer;

  std::string configFilename;
  ServerInfo info;

}; /* struct Server::Impl */

Server::Impl::Impl() {
  // mailboxes
  raft.bindMailboxes(
      config.getMailbox(),
      identityTransformer.getMailbox(),
      rpcSender.getMailbox(),
      timer.getMailbox());
  rpcReceiver.bindMailboxes(raft.getMailbox());
  identityTransformer.bindMailboxes(raft.getMailbox());

}

} /* namespace quintet */

namespace quintet {

Server::Server(const std::string &filename) : pImpl(std::make_unique<Impl>()) {
  pImpl->configFilename = filename;
  pImpl->info = pImpl->config.dispatch(tag::LoadConfig(), filename);
  auto local = pImpl->info.get_local().toString();
  Logger::instance().addId(local);
  pImpl->rpcSender.dispatch(tag::ConfigSender(), pImpl->info.get_srvList());
}
Server::~Server() = default;

void Server::start() {
  pImpl->raft.start(pImpl->configFilename);
  auto local = pImpl->info.get_local().toString();
  pImpl->rpcReceiver.asyncRun(
      std::strtol(
          local.substr(local.find(':'), local.size()).c_str(), nullptr, 10));
}

void Server::shutdown() {
  pImpl->raft.shutdown();
  pImpl->rpcReceiver.shutdown();
}

void Server::wait() {

}

} /* namespace quintet */