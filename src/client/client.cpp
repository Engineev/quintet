#include "client/client.h"

#include <atomic>
#include <memory>
#include <mutex>

#include <grpc++/create_channel.h>

#include "common/config.h"
#include "common/rpc/external.grpc.pb.h"

namespace quintet {

struct Client::Impl {
  ServerInfo info;

  std::string identifier;
  std::atomic_uint64_t cmdSerialNum{1};
  std::uint64_t cmdReceivedNum{0};

  ServerId curLeader;

  class GRpcNode {
  public:
    explicit GRpcNode(const ServerId &srv);
    GEN_COPY(GRpcNode, delete);
    GEN_MOVE(GRpcNode, default);
    ~GRpcNode() = default;

    rpc::RegisterClientReply registerClient();

  private:
    std::unique_ptr<rpc::External::Stub> stub;
  } rpc;

  void registerClient() {
    auto srvList = info.get_srvList();
    ServerId curSrv = srvList.front();
    while (true) {
      GRpcNode curRpc(curSrv);
      auto res = curRpc.registerClient();
      if (res.status() == rpc::Status::OK) {
        rpc = std::move(curRpc);
        identifier = res.clientid();
        return;
      }
      if (res.status() != rpc::Status::NOT_LEADER)
        throw ;
      assert(!res.leaderhint().empty());
      curSrv = ServerId(res.leaderhint());
    }
  }

}; /* struct Client::Impl */

// Impl::GRpcNode

Client::Impl::GRpcNode::GRpcNode(const ServerId &srv)
    : stub(rpc::External::NewStub(grpc::CreateChannel(
    srv.toString(), grpc::InsecureChannelCredentials()))) {}

rpc::RegisterClientReply Client::Impl::GRpcNode::registerClient() {
  rpc::RegisterClientReply res;
  grpc::ClientContext ctx;
  stub->RegisterClient(&ctx, rpc::Empty(), &res);
  return res;
}

// Impl


} // namespace quintet

namespace quintet {

Client::Client(const std::string &filename) {
  pImpl->info.load(filename);
  pImpl->registerClient();
}

GEN_PIMPL_DTOR(Client);

Object Client::callImpl(ClientContext ctx, const std::string &opName,
                        const std::string &args) {

  throw;
}

} // namespace quintet