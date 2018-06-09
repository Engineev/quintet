#include "raft/service/rpc/RpcClients.h"

#include <grpcpp/grpcpp.h>

#include "raft/service/rpc/RaftRpc.grpc.pb.h"

namespace quintet {
namespace rpc {

RpcClients::RpcClients() : pImpl(std::make_unique<RpcClients::Impl>()) {}

RpcClients::~RpcClients() = default;

struct RpcClients::Impl {
  std::unique_ptr<RaftRpc::Stub> stub;
  grpc::CompletionQueue cq;

  struct AsyncClientCall {
    PbReply reply;
    grpc::ClientContext context;
    grpc::Status status;
    boost::promise<std::pair<Term, bool>> prm;
    std::unique_ptr<grpc::ClientAsyncResponseReader<PbReply>> response;
  };

  boost::future<std::pair<Term, bool>>
  asyncCallRpcAppendEntries(ServerId target, Term term, ServerId leaderId,
                            std::size_t prevLogIdx, Term prevLogTerm,
                            std::vector<LogEntry> logEntries,
                            std::size_t commitIdx) {
    AppendEntriesMessage request;
    // TODO: set
    auto call = new AsyncClientCall;
    auto res = call->prm.get_future();
    call->response = stub->AsyncAppendEntries(&call->context, request, &cq);
    call->response->Finish(&call->reply, &call->status, (void*)call);
    return res;
  };

  std::pair<Term, bool>
  callRpcAppendEntries(ServerId target, Term term, ServerId leaderId,
                       std::size_t prevLogIdx, Term prevLogTerm,
                       std::vector<LogEntry> logEntries, std::size_t commitIdx) {
    return asyncCallRpcAppendEntries(target, term, leaderId, prevLogIdx, prevLogTerm,
    std::move(logEntries), commitIdx).get();
  };

  void run() {
    void * tag;
    bool ok = false;
    while (cq.Next(&tag, &ok)) {
      auto call = static_cast<AsyncClientCall*>(tag);
      // TODO
      delete call;
    }
  }

  void shutdown() {
    cq.Shutdown();
  }
};

} // namespace rpc
} // namespace quintet