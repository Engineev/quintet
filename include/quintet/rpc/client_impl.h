#ifndef QUINTET_CLIENT_IMPL_H
#define QUINTET_CLIENT_IMPL_H

#include <chrono>

#include <boost/thread/thread_only.hpp>
#include <boost/thread/future.hpp>

#include <grpcpp/grpcpp.h>

#include "client_context.h"
#include "common.h"
#include "rpc/Quintet.grpc.pb.h"
#include "rpc/error.h"
#include "rpc/conversion.h"
#include "misc/object.h"

namespace quintet {
namespace rpc {

template <class PbReply, class Result, class Rpc>
class ClientImpl {
public:
  explicit ClientImpl(const ServerId & target) {
    stub = Rpc::NewStub(
        grpc::CreateChannel(target.toString(),
                            grpc::InsecureChannelCredentials()));
    asyncRun();
  }

  ~ClientImpl() {
    stop();
  }

  void run() {
    void *tag;
    bool ok = false;
    while (cq.Next(&tag, &ok)) {
      std::unique_ptr<AsyncClientCall> call(
          static_cast<AsyncClientCall *>(tag));
      if (!call->status.ok()) {
        call->prm.set_exception(
            rpc::Error(std::to_string((int)call->status.error_code()) + ", " +
                call->status.error_message()));
        return;
      }
      auto leader = rpc::convertServerId(call->reply.leaderid());
      if (!leader) {
        call->prm.set_exception(rpc::NotLeader(leader));
        return;
      }
      Object obj;
      obj.getMutable_buffer() = call->reply.ret();
      call->prm.set_value(obj);
    }
  }

  void asyncRun() {
    runningThread = boost::thread([this] { run(); });
  }

  void stop() {
    cq.Shutdown();
    runningThread.join();
  }
  

protected:
  template <class Request>
  boost::future<Result> asyncCallImpl(ClientContext ctx, Request request) {
    auto call = new AsyncClientCall;
    call->context.set_deadline(std::chrono::system_clock::now()
                               + std::chrono::milliseconds(ctx.getTimeout()));
    auto res = call->prm.get_future();
    call->response = stub->Asynccall(&call->context, std::move(request), &cq);
    call->response->Finish(&call->reply, &call->status, (void *)call);
    return res;
  }

private:
  std::unique_ptr<typename Rpc::Stub> stub;
  grpc::CompletionQueue cq;
  boost::thread runningThread;

  struct AsyncClientCall {
    PbReply reply;
    grpc::ClientContext context;
    grpc::Status status;
    boost::promise<Result> prm;
    std::unique_ptr<grpc::ClientAsyncResponseReader<PbReply>> response;
  };

};

} // namespace rpc
} // namespace quintet

#endif //QUINTET_CLIENT_IMPL_H
