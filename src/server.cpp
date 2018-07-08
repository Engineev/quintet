#include "server.h"

#include <unordered_map>
#include <functional>

#include "server_info.h"
#include "rpc/rpc_server.h"
#include "raft/raft.h"


/* -------------------------- ServerImpl ------------------------------------ */

namespace quintet {

struct Server::Impl {
  Impl(const std::string & filename) : raft(info) {
    info.load(filename);
  }

  raft::Raft raft;
  std::unordered_map<std::string, std::function<boost::any(std::string)>> fs;
  ServerInfo info;
  rpc::RpcServer rpc;

  void bind(const std::string &name, std::function<boost::any(std::string)> f);

}; // struct Server::Impl

void Server::Impl::bind(const std::string &name,
                        std::function<boost::any(std::string)> f) {
  fs.emplace(name, std::move(f));
}

} // namespace quintet

namespace quintet {

Server::Server(const std::string &filename)
    : pImpl(std::make_unique<Impl>(filename)) {}
Server::~Server() = default;

Server &Server::bindImpl(const std::string &name,
                         std::function<boost::any(std::string)> f) {
  pImpl->bind(name, std::move(f));
  return *this;
}

void Server::Start() {
  pImpl->rpc.start(pImpl->info.get_local().get_port());
  throw ;
}
void Server::Wait() { throw ; }
void Server::Shutdown() { throw ; }


} // namespace quintet