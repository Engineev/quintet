#include "raft/service/rpc/RpcClients.h"

#include <stdexcept>
#include <unordered_map>

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/shared_lock_guard.hpp>
#include <boost/thread/lock_guard.hpp>

#include <grpcpp/grpcpp.h>

#include "service/rpc/Conversion.h"
#include "service/rpc/RaftRpc.grpc.pb.h"

namespace quintet {
namespace rpc {

RpcClients::RpcClients() : pImpl(std::make_unique<RpcClients::Impl>()) {}

struct RpcClients::Impl {
  struct Node {
    std::unique_ptr<boost::shared_mutex> m;
    std::unique_ptr<RaftRpc::Stub> stub;

    Node() = default;
    Node(Node &&) = default;
  };

  std::unordered_map<ServerId, Node> stubs;
  grpc::CompletionQueue cq;
  boost::thread runningThread;

  struct AsyncClientCall {
    PbReply reply;
    std::shared_ptr<grpc::ClientContext> context;
    grpc::Status status;
    boost::promise<std::pair<Term, bool>> prm;
    std::unique_ptr<grpc::ClientAsyncResponseReader<PbReply>> response;
  };

  void createStubs(const std::vector<ServerId> &srvs) {
    for (auto &srv : srvs) {
      Node node;
      node.m = std::make_unique<boost::shared_mutex>();
      node.stub = RaftRpc::NewStub(grpc::CreateChannel(
          srv.addr + ":" + std::to_string(srv.port),
          grpc::InsecureChannelCredentials()));
      stubs.emplace(srv, std::move(node));
    }
  }

  boost::future<Reply>
  asyncCallRpcAppendEntries(const ServerId &target,
                            std::shared_ptr<grpc::ClientContext> ctx,
                            const AppendEntriesMessage &msg) {
    PbAppendEntriesMessage request = convertAppendEntriesMessage(msg);

    auto call = new AsyncClientCall;
    call->context = std::move(ctx);
    auto res = call->prm.get_future();
    auto & node = stubs.at(target);
    boost::shared_lock_guard<boost::shared_mutex> lk(*node.m);
    auto &stub = node.stub;
    call->response = stub->AsyncAppendEntries(&*call->context, request, &cq);
    call->response->Finish(&call->reply, &call->status, (void *)call);
    return res;
  };

  boost::future<Reply>
  asyncCallRequestVote(const ServerId &target,
                       std::shared_ptr<grpc::ClientContext> ctx,
                       const RequestVoteMessage &msg) {
    PbRequestVoteMessage request = convertRequestVoteMessage(msg);
    auto call = new AsyncClientCall;
    auto res = call->prm.get_future();
    call->context = std::move(ctx);
    auto & node = stubs.at(target);
    boost::shared_lock_guard<boost::shared_mutex> lk(*node.m);
    auto &stub = node.stub;
    call->response = stub->AsyncRequestVote(&*call->context, request, &cq);
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
        call->prm.set_exception(
            RpcError(std::to_string((int)call->status.error_code()) + ", "
                         + call->status.error_message()));
        return;
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

  void reset(const ServerId & srv) {
    auto & node = stubs.at(srv);
    boost::lock_guard<boost::shared_mutex> lk(*node.m);
    node.stub = RaftRpc::NewStub(grpc::CreateChannel(
        srv.addr + ":" + std::to_string(srv.port),
        grpc::InsecureChannelCredentials()));;
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

void RpcClients::reset(const ServerId & srv) {
  pImpl->reset(srv);
}

boost::future<std::pair<Term, bool>>
RpcClients::asyncCallRpcAppendEntries(const ServerId &target,
                                      std::shared_ptr<grpc::ClientContext> ctx,
                                      const AppendEntriesMessage &msg) {
  return pImpl->asyncCallRpcAppendEntries(target, std::move(ctx), msg);
}

Reply RpcClients::callRpcAppendEntries(const ServerId &target,
                                       std::shared_ptr<grpc::ClientContext> ctx,
                                       const AppendEntriesMessage &msg) {
  return asyncCallRpcAppendEntries(target, std::move(ctx), msg).get();
}
boost::future<std::pair<Term, bool>>
RpcClients::asyncCallRpcRequestVote(const ServerId &target,
                                    std::shared_ptr<grpc::ClientContext> ctx,
                                    const RequestVoteMessage &msg) {
  return pImpl->asyncCallRequestVote(target, std::move(ctx), msg);
}
Reply RpcClients::callRpcRequestVote(const ServerId &target,
                                     std::shared_ptr<grpc::ClientContext> ctx,
                                     const RequestVoteMessage &msg) {
  return asyncCallRpcRequestVote(target, std::move(ctx), msg).get();
}

} // namespace rpc
} // namespace quintet