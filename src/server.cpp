#include "server.h"

#include <unordered_map>
#include <functional>

#include "server_info.h"
#include "rpc/rpc_server.h"
#include "raft/raft.h"


/* -------------------------- ServerImpl ------------------------------------ */

namespace quintet {

struct Server::Impl {
  raft::Raft raft;
  std::unordered_map<std::string, std::function<boost::any(std::string)>> fs;
  ServerInfo info;
  rpc::RpcServer rpc;


  void configure(const std::string & filename) {
    raft.Configure(filename);
    info.load(filename);
    rpc.start(info.get_local().get_port());
  }

  void start() { throw; }


  void bind(const std::string &name, std::function<boost::any(std::string)> f);

}; // struct Server::Impl

void Server::Impl::bind(const std::string &name,
                        std::function<boost::any(std::string)> f) {
  fs.emplace(name, std::move(f));
}

} // namespace quintet

namespace quintet {

Server::Server() : pImpl(std::make_unique<Impl>()) {}
Server::~Server() = default;

Server &Server::bindImpl(const std::string &name,
                         std::function<boost::any(std::string)> f) {
  pImpl->bind(name, std::move(f));
  return *this;
}

void Server::Configure() {

}
void Server::Start() { pImpl->start(); }
void Server::Wait() {

}
void Server::Shutdown() {

}

} // namespace quintet