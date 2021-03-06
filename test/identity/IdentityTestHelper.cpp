#include "IdentityTestHelper.h"

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include "service/rpc/Conversion.h"
#include "service/rpc/RaftRpc.grpc.pb.h"
#include "service/rpc/RpcDefs.h"
#include "service/rpc/RpcClient.h"

namespace quintet {
namespace test {

std::vector<std::unique_ptr<Raft>>
IdentityTestHelper::makeServers(std::size_t num) {
  assert(num <= 5);
  std::vector<std::unique_ptr<Raft>> res;
  for (int i = 0; i < (int)num; ++i) {
    auto tmp = std::make_unique<Raft>();
    tmp->Configure(std::string(CMAKE_SOURCE_DIR) +
                   "/test/RaftConfig/RaftConfig" + std::to_string(i) + ".json");
    res.emplace_back(std::move(tmp));
  }
  return res;
}

void IdentityTestHelper::sendHeartBeat(const std::vector<ServerId> &srvs,
                                       const ServerId &local,
                                       Term currentTerm) {
  using namespace quintet::rpc;
  for (auto &srv : srvs) {
    if (srv == local)
      continue;
    RpcClient client(grpc::CreateChannel(srv.addr + ":" + std::to_string(srv.port),
                                         grpc::InsecureChannelCredentials()));
    auto ctx = std::make_shared<grpc::ClientContext>();
    quintet::AppendEntriesMessage msg;
    msg.term = currentTerm;
    client.callRpcAppendEntries(ctx, msg);
  }
}

} // namespace test
} // namespace quintet