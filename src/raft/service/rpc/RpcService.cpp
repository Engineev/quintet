#include "service/rpc/RpcService.h"

#include <functional>
#include <utility>
#include <vector>

#include <boost/atomic/atomic.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/thread.hpp>

#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>

#include "service/rpc/RaftRpc.grpc.pb.h"
#include "service/rpc/Conversion.h"

/* ---------------------------- RaftRpc ------------------------------------- */

namespace quintet {
namespace rpc {

class RaftRpcImpl final : public RaftRpc::Service {
  using AppendEntriesFunc =
      std::function<std::pair<Term, bool>(AppendEntriesMessage)>;
  using RequestVoteFunc =
      std::function<std::pair<Term, bool>(RequestVoteMessage)>;

public:
  void bindAppendEntries(AppendEntriesFunc f) { appendEntries = std::move(f); }
  void bindRequestVote(RequestVoteFunc f) { requestVote = std::move(f); }

  grpc::Status AppendEntries(grpc::ServerContext *context,
                             const PbAppendEntriesMessage *request,
                             PbReply *response) override {
    AppendEntriesMessage msg = convertAppendEntriesMessage(*request);
    response->CopyFrom(convertReply(appendEntries(std::move(msg))));
    return grpc::Status::OK;
  }

  grpc::Status RequestVote(grpc::ServerContext *context,
                           const PbRequestVoteMessage *request,
                           PbReply *response) override {
    RequestVoteMessage msg = convertRequestVoteMessage(*request);
    response->CopyFrom(convertReply(requestVote(std::move(msg))));
    return grpc::Status::OK;
  }

private:
  AppendEntriesFunc appendEntries;
  RequestVoteFunc requestVote;
};

} // namespace rpc
} // namespace quintet

/* --------------------------- RpcService ----------------------------------- */

namespace quintet {
namespace rpc {

RpcService::RpcService() : pImpl(std::make_unique<Impl>()) {}

RpcService::~RpcService() = default;

struct RpcService::Impl {
  RaftRpcImpl service;
  std::unique_ptr<grpc::Server> server;
  boost::thread runningThread;

  boost::mutex stopping;
  boost::atomic<std::size_t> numRpcRemaining{0};
  boost::condition_variable modifiedNumRpcRemaining;

  // sync pause()/resume() and rpc
  boost::shared_mutex rpcing;
  boost::mutex paused;

  void asyncRun(Port port) {
    std::string addr = "0.0.0.0:" + std::to_string(port);
    grpc::ServerBuilder builder;
    builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    server = builder.BuildAndStart();
    runningThread = boost::thread([this] { server->Wait(); });
  }

  std::shared_ptr<void> preRpc() {
    ++numRpcRemaining;
    boost::unique_lock<boost::mutex> plk(paused);
    boost::shared_lock<boost::shared_mutex> lk(rpcing);
    plk.unlock();

    return std::shared_ptr<void>(nullptr, [this](void *) {
      --numRpcRemaining;
      modifiedNumRpcRemaining.notify_all();
    });
  }

  void bindAppendEntries(
      std::function<std::pair<Term, bool>(AppendEntriesMessage)> f) {
    service.bindAppendEntries([this, f](AppendEntriesMessage msg) {
      auto defer = preRpc();
      return f(std::move(msg));
    });
  }

  void
  bindRequestVote(std::function<std::pair<Term, bool>(RequestVoteMessage)> f) {
    service.bindRequestVote([this, f](RequestVoteMessage msg) {
      auto defer = preRpc();
      return f(std::move(msg));
    });
  }

  void pause() {
    paused.lock();
    boost::unique_lock<boost::shared_mutex> lk(rpcing);
  }

  void resume() { paused.unlock(); }

  void stop() {
    boost::unique_lock<boost::mutex> lk(stopping);
    modifiedNumRpcRemaining.wait(lk, [this] { return !numRpcRemaining; });
    if (server)
      server->Shutdown();
  }

}; // struct RpcService::Impl

} // namespace rpc
} // namespace quintet

namespace quintet {
namespace rpc {

void RpcService::asyncRun(Port port) { pImpl->asyncRun(port); }

void RpcService::pause() { pImpl->pause(); }

void RpcService::resume() { pImpl->resume(); }

void RpcService::stop() { pImpl->stop(); }

void RpcService::bindAppendEntries(
    std::function<std::pair<Term, bool>(AppendEntriesMessage)> f) {
  pImpl->bindAppendEntries(std::move(f));
}
void RpcService::bindRequestVote(
    std::function<std::pair<Term, bool>(RequestVoteMessage)> f) {
  pImpl->bindRequestVote(std::move(f));
}

} // namespace rpc
} // namespace quintet