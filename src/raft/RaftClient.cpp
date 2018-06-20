#include "RaftClient.h"

#include <grpcpp/grpcpp.h>

#include "service/rpc/Conversion.h"
#include "service/rpc/RaftRpc.grpc.pb.h"

namespace quintet {

struct RaftClient::Impl {
  std::unique_ptr<rpc::RaftRpc::Stub> stub;
  grpc::CompletionQueue cq;
  boost::thread runningThread;

  struct AsyncClientCall {
    rpc::PbAddLogReply reply;
    std::shared_ptr<grpc::ClientContext> context;
    grpc::Status status;
    boost::promise<AddLogReply> prm;
    std::unique_ptr<
        grpc::ClientAsyncResponseReader<rpc::PbAddLogReply>> response;
  };

  void run() {
    void *tag;
    bool ok = false;
    while (cq.Next(&tag, &ok)) {
      std::unique_ptr<AsyncClientCall> call(
          static_cast<AsyncClientCall *>(tag));
      if (!call->status.ok()) {
        call->prm.set_exception(
            rpc::RpcError(std::to_string((int)call->status.error_code()) + ", " +
                call->status.error_message()));
        return;
      }
      call->prm.set_value(convertAddLogReply(call->reply));
    }
  }

  void asyncRun() {
    runningThread = boost::thread([this] { run(); });
  }

  boost::future<AddLogReply>
  asyncCallRpcAddLog(std::shared_ptr<grpc::ClientContext> ctx,
                     const AddLogMessage &msg) {
    rpc::PbAddLogMessage request = rpc::convertAddLogMessage(msg);

    auto call = new AsyncClientCall;
    call->context = std::move(ctx);
    auto res = call->prm.get_future();
    call->response = stub->AsyncAddLog(&*call->context, request, &cq);
    call->response->Finish(&call->reply, &call->status, (void *)call);
    return res;
  }
}; // struct RaftClient::Impl

RaftClient::RaftClient(std::shared_ptr<grpc::Channel> channel)
    : pImpl(std::make_unique<Impl>()) {
  pImpl->stub = rpc::RaftRpc::NewStub(channel);
  pImpl->asyncRun();
}

RaftClient::RaftClient(const ServerId &target)
    : pImpl(std::make_unique<Impl>()) {
  pImpl->stub = rpc::RaftRpc::NewStub(
      grpc::CreateChannel(target.toString(),
                          grpc::InsecureChannelCredentials()));
  pImpl->asyncRun();
}

RaftClient::RaftClient(RaftClient &&) noexcept = default;

RaftClient::~RaftClient() {
  pImpl->cq.Shutdown();
  pImpl->runningThread.join();
}

} // namespace quintet

namespace quintet {

AddLogReply RaftClient::callRpcAddLog(std::shared_ptr<grpc::ClientContext> ctx,
                                      const quintet::AddLogMessage &msg) {
  return pImpl->asyncCallRpcAddLog(std::move(ctx), msg).get();
}
boost::future<AddLogReply> RaftClient::asyncCallRpcAddLog(
    std::shared_ptr<grpc::ClientContext> ctx, const AddLogMessage &msg) {
  return pImpl->asyncCallRpcAddLog(std::move(ctx), msg);
}

} // namespace quintet