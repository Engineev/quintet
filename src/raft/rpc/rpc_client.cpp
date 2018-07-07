#include "raft/rpc/rpc_client.h"

#include "rpc/client_impl.h"
#include "raft/raft_common.h"
#include "raft/rpc/common.h"
#include "raft/rpc/conversion.h"
#include "raft/rpc/raft.grpc.pb.h"

namespace quintet {
namespace raft {
namespace rpc {

class Client::Impl
  : public quintet::rpc::ClientImpl<PbReply, Reply, RaftRpc> {
public:
  explicit Impl(const ServerId &target)
    : quintet::rpc::ClientImpl<PbReply, Reply, RaftRpc>(target) {
    startImpl(boost::thread([this] { run(); }));
  }

  boost::future<Reply>
  asyncAppendEntries(ClientContext ctx, const AppendEntriesMessage & msg) {
    return asyncCallImpl(ctx, convertAppendEntriesMessage(msg),
                         &RaftRpc::Stub::AsyncAppendEntries);
  }

  boost::future<Reply>
  asyncRequestVote(ClientContext ctx, const RequestVoteMessage & msg) {
    return asyncCallImpl(ctx, convertRequestVoteMessage(msg),
                         &RaftRpc::Stub::AsyncRequestVote);
  }

private:
  void run() {
    runImpl([] (boost::promise<Reply> & prm, const PbReply & reply) {
      prm.set_value(convertReply(reply));
    });
  }
}; // class Client::Impl

} // namespace rpc
} // namespace raft
} // namespace quintet

namespace quintet {
namespace raft {
namespace rpc {

Client::Client(const ServerId & target)
    : pImpl(std::make_unique<Impl>(target)) {}
Client::~Client() = default;

Reply Client::callRpcAppendEntries(ClientContext ctx,
                                   const AppendEntriesMessage &msg) {
  return pImpl->asyncAppendEntries(ctx, msg).get();
}
boost::future<Reply>
Client::asyncCallRpcAppendEntries(ClientContext ctx,
                                  const AppendEntriesMessage &msg) {
  return pImpl->asyncAppendEntries(ctx, msg);
}
Reply Client::callRpcRequestVote(ClientContext ctx,
                                 const RequestVoteMessage &msg) {
  return pImpl->asyncRequestVote(ctx, msg).get();
}
boost::future<Reply>
Client::asyncCallRpcRequestVote(ClientContext ctx,
                                const RequestVoteMessage &msg) {
  return pImpl->asyncRequestVote(ctx, msg);
}

} // namespace rpc
} // namespace raft
} // namespace quintet

