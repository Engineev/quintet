#ifndef QUINTET_RPCCLIENT_H
#define QUINTET_RPCCLIENT_H

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include <boost/thread/future.hpp>
#include <grpcpp/client_context.h>

#include "RaftDefs.h"
#include "RpcDefs.h"

namespace quintet {
namespace rpc {

class RpcClient {
public:
  explicit RpcClient(std::shared_ptr<grpc::Channel> channel);

  ~RpcClient();

  RpcClient(RpcClient &&);
  RpcClient(const RpcClient &) = delete;

  Reply callRpcAppendEntries(std::shared_ptr<grpc::ClientContext> ctx,
                             const AppendEntriesMessage &msg);

  boost::future<Reply>
  asyncCallRpcAppendEntries(std::shared_ptr<grpc::ClientContext> ctx,
                            const AppendEntriesMessage &msg);

  Reply callRpcRequestVote(std::shared_ptr<grpc::ClientContext> ctx,
                           const RequestVoteMessage &msg);

  boost::future<Reply>
  asyncCallRpcRequestVote(std::shared_ptr<grpc::ClientContext> ctx,
                          const RequestVoteMessage &msg);

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
}; // class RpcClients
} // namespace rpc
} // namespace quintet

#endif //QUINTET_RPCCLIENT_H
