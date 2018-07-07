#include "rpc/rpc_server.h"

#include <functional>

#include <boost/thread/thread_only.hpp>

#include <grpc++/server.h>
#include <grpc++/server_builder.h>

#include "common.h"
#include "rpc/quintet.grpc.pb.h"

/* -------------------------- ExternalImpl ---------------------------------- */

namespace quintet {
namespace rpc {

class ExternalImpl : public External::Service {
public:
  using AddLogFunc = std::function<std::pair<bool, ServerId>(BasicLogEntry)>;

  void bindAddLog(AddLogFunc f) { addLog = std::move(f); }

  grpc::Status call(grpc::ServerContext *context,
                    const PbExternalMessage *request,
                    PbExternalReply *response) override;

private:
  AddLogFunc addLog;
}; // class ExternalImpl

} // namespace rpc
} // namespace quintet

/* -------------------------------------------------------------------------- */

namespace quintet {
namespace rpc {

struct RpcServer::Impl {
  ExternalImpl service;
  std::unique_ptr<grpc::Server> server;
  boost::thread runningThread;

}; // struct RpcServer::Impl

} // namespace rpc
} // namespace quintet

/* -------------------------------------------------------------------------- */

namespace quintet {
namespace rpc {

GEN_PIMPL_CTOR(RpcServer);
RpcServer::~RpcServer() = default;

void RpcServer::start(Port port) {
  std::string addr = "0.0.0.0:" + std::to_string(port);
  grpc::ServerBuilder builder;
  builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
  builder.RegisterService(&pImpl->service);
  pImpl->server = builder.BuildAndStart();
  pImpl->runningThread = boost::thread([this] { pImpl->server->Wait(); });
}

void RpcServer::bindAddLog(
    std::function<std::pair<bool, ServerId>(BasicLogEntry)> f) {
  pImpl->service.bindAddLog(std::move(f));
}

} // namespace rpc
} // namespace quintet