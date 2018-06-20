#ifndef QUINTET_RAFTCLIENT_H
#define QUINTET_RAFTCLIENT_H

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include <boost/thread/future.hpp>
#include <grpcpp/client_context.h>

#include "RaftDefs.h"
#include "service/rpc/RpcDefs.h"

namespace quintet {

class RaftClient {
public:
  explicit RaftClient(std::shared_ptr<grpc::Channel> channel);

  explicit RaftClient(const ServerId & target);

  ~RaftClient();

  RaftClient(RaftClient &&) noexcept;
  RaftClient(const RaftClient &) = delete;

  AddLogReply callRpcAddLog(std::shared_ptr<grpc::ClientContext> ctx,
                      const AddLogMessage & msg);

  boost::future<AddLogReply>
  asyncCallRpcAddLog(std::shared_ptr<grpc::ClientContext> ctx,
                     const AddLogMessage & msg);

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;

}; // class RaftClient

} // namespace quintet

#endif //QUINTET_RAFTCLIENT_H
