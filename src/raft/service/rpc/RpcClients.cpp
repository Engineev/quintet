#include "raft/service/rpc/RpcClients.h"

#include <unordered_map>

#include <grpcpp/grpcpp.h>

#include "service/rpc/RaftRpc.grpc.pb.h"
#include "service/rpc/Conversion.h"

namespace quintet {
namespace rpc {

RpcClients::RpcClients() : pImpl(std::make_unique<RpcClients::Impl>()) {}

struct RpcClients::Impl {
  std::unordered_map<ServerId, std::unique_ptr<RaftRpc::Stub>> stubs;
  grpc::CompletionQueue cq;
  boost::thread runningThread;

  struct AsyncClientCall {
    PbReply reply;
    grpc::ClientContext context;
    grpc::Status status;
    boost::promise<std::pair<Term, bool>> prm;
    std::unique_ptr<grpc::ClientAsyncResponseReader<PbReply>> response;
  };

  void createStubs(const std::vector<ServerId> &srvs) {
    for (auto &srv : srvs) {
      stubs.emplace(srv, RaftRpc::NewStub(grpc::CreateChannel(
                             srv.addr + ":" + std::to_string(srv.port),
                             grpc::InsecureChannelCredentials())));
    }
  }

  // TODO: context !!
  boost::future<Reply>
  asyncCallRpcAppendEntries(const ServerId &target, grpc::ClientContext &ctx,
                            const AppendEntriesMessage &msg) {
    PbAppendEntriesMessage request = convertAppendEntriesMessage(msg);

    auto call = new AsyncClientCall;
    auto res = call->prm.get_future();
    auto &stub = stubs.at(target);
    call->response = stub->AsyncAppendEntries(&call->context, request, &cq);
    call->response->Finish(&call->reply, &call->status, (void *)call);
    return res;
  };

  boost::future<Reply> asyncCallRequestVote(const ServerId &target,
                                            grpc::ClientContext &ctx,
                                            const RequestVoteMessage &msg) {
    PbRequestVoteMessage request = convertRequestVoteMessage(msg);
    auto call = new AsyncClientCall;
    auto res = call->prm.get_future();
    auto &stub = stubs.at(target);
    call->response = stub->AsyncRequestVote(&call->context, request, &cq);
    call->response->Finish(&call->reply, &call->status, (void *)call);
    return res;
  }

  void run() {
    void *tag;
    bool ok = false;
    while (cq.Next(&tag, &ok)) {
      std::unique_ptr<AsyncClientCall> call(
          static_cast<AsyncClientCall *>(tag));
      if (!call->status.ok()) {
        throw;
      }
      call->prm.set_value(
          std::make_pair(call->reply.term(), call->reply.ans()));
    }
  }

  void asyncRun() {
    runningThread = boost::thread([this] { run(); });
  }

  void stop() {
    cq.Shutdown();
    runningThread.join();
  }
};

RpcClients::~RpcClients() { pImpl->stop(); }

} // namespace rpc
} // namespace quintet

namespace quintet {
namespace rpc {

void RpcClients::createStubs(const std::vector<ServerId> &srvs) {
  pImpl->createStubs(srvs);
}

void RpcClients::asyncRun() { pImpl->asyncRun(); }

void RpcClients::stop() { pImpl->stop(); }

boost::future<std::pair<Term, bool>>
RpcClients::asyncCallRpcAppendEntries(const ServerId &target,
                                      grpc::ClientContext &ctx,
                                      const AppendEntriesMessage &msg) {
  return pImpl->asyncCallRpcAppendEntries(target, ctx, msg);
}

Reply RpcClients::callRpcAppendEntries(const ServerId &target,
                                       grpc::ClientContext &ctx,
                                       const AppendEntriesMessage &msg) {
  return asyncCallRpcAppendEntries(target, ctx, msg).get();
}
boost::future<std::pair<Term, bool>>
RpcClients::asyncCallRpcRequestVote(const ServerId &target,
                                    grpc::ClientContext &ctx,
                                    const RequestVoteMessage &msg) {
  return pImpl->asyncCallRequestVote(target, ctx, msg);
}
Reply RpcClients::callRpcRequestVote(const ServerId &target,
                                     grpc::ClientContext &ctx,
                                     const RequestVoteMessage &msg) {
  return asyncCallRpcRequestVote(target, ctx, msg).get();
}

} // namespace rpc
} // namespace quintet