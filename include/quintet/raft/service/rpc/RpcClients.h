#ifndef QUINTET_RPCCLIENTS_H
#define QUINTET_RPCCLIENTS_H

#include <cstddef>
#include <memory>
#include <vector>
#include <utility>

#include <boost/thread/future.hpp>
#include <grpcpp/client_context.h>

#include "RaftDefs.h"
#include "RpcDefs.h"

namespace quintet {
namespace rpc {

class RpcClients {
public:
  RpcClients();

  ~RpcClients();

  void createStubs(const std::vector<ServerId> & srvs);

  Reply callRpcAppendEntries(const ServerId & target, grpc::ClientContext & ctx, const AppendEntriesMessage & msg);

  boost::future<Reply>
  asyncCallRpcAppendEntries(const ServerId & target, grpc::ClientContext & ctx, const AppendEntriesMessage & msg);

  Reply callRpcRequestVote(const ServerId & target, grpc::ClientContext & ctx, const RequestVoteMessage & msg);
//
  boost::future<Reply>
  asyncCallRpcRequestVote(const ServerId & target, grpc::ClientContext & ctx, const RequestVoteMessage & msg);

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
}; // class RpcClients
} // namespace rpc
} // namespace quintet

#endif // QUINTET_RPCCLIENTS_H
