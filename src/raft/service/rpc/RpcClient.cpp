#include "service/rpc/RpcClient.h"

#include <grpcpp/grpcpp.h>

#include "service/rpc/Conversion.h"
#include "service/rpc/RaftRpc.grpc.pb.h"

namespace quintet {
namespace rpc {

struct RpcClient::Impl {
  std::unique_ptr<RaftRpc::Stub> stub;
  grpc::CompletionQueue cq;
  boost::thread runningThread;

  struct AsyncClientCall {
    PbReply reply;
    std::shared_ptr<grpc::ClientContext> context;
    grpc::Status status;
    boost::promise<std::pair<Term, bool>> prm;
    std::unique_ptr<grpc::ClientAsyncResponseReader<PbReply>> response;
  };

  void run() {
    void *tag;
    bool ok = false;
    while (cq.Next(&tag, &ok)) {
      std::unique_ptr<AsyncClientCall> call(
          static_cast<AsyncClientCall *>(tag));
      if (!call->status.ok()) {
        call->prm.set_exception(
            RpcError(std::to_string((int)call->status.error_code()) + ", " +
                     call->status.error_message()));
        return;
      }
      call->prm.set_value(
          std::make_pair(call->reply.term(), call->reply.ans()));
    }
  }

  void asyncRun() {
    runningThread = boost::thread([this] { run(); });
  }

  boost::future<Reply>
  asyncCallRpcAppendEntries(std::shared_ptr<grpc::ClientContext> ctx,
                            const AppendEntriesMessage &msg) {
    PbAppendEntriesMessage request = convertAppendEntriesMessage(msg);

    auto call = new AsyncClientCall;
    call->context = std::move(ctx);
    auto res = call->prm.get_future();
    call->response = stub->AsyncAppendEntries(&*call->context, request, &cq);
    call->response->Finish(&call->reply, &call->status, (void *)call);
    return res;
  }

  boost::future<Reply>
  asyncCallRequestVote(std::shared_ptr<grpc::ClientContext> ctx,
                       const RequestVoteMessage &msg) {
    PbRequestVoteMessage request = convertRequestVoteMessage(msg);
    auto call = new AsyncClientCall;
    auto res = call->prm.get_future();
    call->context = std::move(ctx);
    call->response = stub->AsyncRequestVote(&*call->context, request, &cq);
    call->response->Finish(&call->reply, &call->status, (void *)call);
    return res;
  }

}; // struct RpcClient::Impl

RpcClient::RpcClient(std::shared_ptr<grpc::Channel> channel)
    : pImpl(std::make_unique<Impl>()) {
  pImpl->stub = RaftRpc::NewStub(channel);
  pImpl->asyncRun();
}

RpcClient::~RpcClient() {
  pImpl->cq.Shutdown();
  pImpl->runningThread.join();
}

} // namespace rpc
} // namespace quintet

namespace quintet {
namespace rpc {

Reply RpcClient::callRpcAppendEntries(std::shared_ptr<grpc::ClientContext> ctx,
                                      const AppendEntriesMessage &msg) {
  return pImpl->asyncCallRpcAppendEntries(std::move(ctx), msg).get();
}

boost::future<Reply>
RpcClient::asyncCallRpcAppendEntries(std::shared_ptr<grpc::ClientContext> ctx,
                                     const AppendEntriesMessage &msg) {
  return pImpl->asyncCallRpcAppendEntries(std::move(ctx), msg);
}
Reply RpcClient::callRpcRequestVote(std::shared_ptr<grpc::ClientContext> ctx,
                                    const RequestVoteMessage &msg) {
  assert(pImpl);
  return pImpl->asyncCallRequestVote(std::move(ctx), msg).get();
}
boost::future<Reply>
RpcClient::asyncCallRpcRequestVote(std::shared_ptr<grpc::ClientContext> ctx,
                                   const RequestVoteMessage &msg) {
  return pImpl->asyncCallRequestVote(std::move(ctx), msg);
}

} // namespace rpc
} // namespace quintet