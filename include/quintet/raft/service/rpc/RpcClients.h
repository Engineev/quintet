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

//  std::pair<Term, bool>
//  callRpcAppendEntries(AppendEntriesMessage msg);

  boost::future<std::pair<Term, bool>>
  asyncCallRpcAppendEntries(ServerId target, grpc::ClientContext & ctx, const AppendEntriesMessage & msg);

//  std::pair<Term, bool> callRpcRequestVote(ServerId target, RequestVoteMessage msg);
//
//  boost::future<std::pair<Term, bool>>
//  asyncCallRpcRequestVote(ServerId target, Term term, ServerId candidateId,
//                          std::size_t lastLogIdx, Term lastLogTerm);

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
}; // class RpcClients
} // namespace rpc
} // namespace quintet

#endif // QUINTET_RPCCLIENTS_H
