#include "raft/rpc/rpc_server.h"

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

#include "raft/rpc/raft.grpc.pb.h"
#include "raft/rpc/conversion.h"
#include "misc/macro.h"

/* ---------------------------- GRPC ---------------------------------------- */

namespace quintet {
namespace raft {
namespace rpc {

class RaftRpcImpl final : public RaftRpc::Service {
  using AppendEntriesFunc = std::function<Reply(AppendEntriesMessage)>;
  using RequestVoteFunc = std::function<Reply(RequestVoteMessage)>;

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
} // namespace raft
} // namespace quintet

/* ------------------------- Server::Impl ----------------------------------- */

namespace quintet {
namespace raft {
namespace rpc {

struct Server::Impl {
  RaftRpcImpl service;
  std::unique_ptr<grpc::Server> server;

  boost::thread runningThread;

  boost::atomic_bool paused{false};
  boost::atomic_size_t numRpcRemaining{0};
  boost::condition_variable cv;
  boost::mutex rpcing;

  std::shared_ptr<void> preRpc() {
    ++numRpcRemaining;
    return std::shared_ptr<void>(nullptr, [this](void *) {
      --numRpcRemaining;
      cv.notify_all();
    });
  }

}; // struct Server::Impl

} // namespace rpc
} // namespace raft
} // namespace quintet

/* --------------------------- Server --------------------------------------- */

namespace quintet {
namespace raft {
namespace rpc {

GEN_PIMPL_CTOR(Server)
Server::~Server() = default;

void Server::bindAppendEntries(std::function<Reply(AppendEntriesMessage)> f) {
  pImpl->service.bindAppendEntries([this, f](AppendEntriesMessage msg) {
    auto defer = pImpl->preRpc();
    boost::unique_lock<boost::mutex> lk(pImpl->rpcing);
    pImpl->cv.wait(lk, [this] { return !pImpl->paused; });
    return f(std::move(msg));
  });
}

void Server::bindRequestVote(std::function<Reply(RequestVoteMessage)> f) {
  pImpl->service.bindRequestVote([this, f](RequestVoteMessage msg) {
    auto defer = pImpl->preRpc();
    boost::unique_lock<boost::mutex> lk(pImpl->rpcing);
    pImpl->cv.wait(lk, [this] { return !pImpl->paused; });
    return f(std::move(msg));
  });
}

void Server::pause() {
  pImpl->paused = true;
}

void Server::resume() {
  pImpl->paused = false;
  pImpl->cv.notify_all();
}

void Server::asyncRun(Port port) {
  std::string addr = "0.0.0.0:" + std::to_string(port);
  grpc::ServerBuilder builder;
  builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
  builder.RegisterService(&pImpl->service);
  pImpl->server = builder.BuildAndStart();
  pImpl->runningThread = boost::thread([this] { pImpl->server->Wait(); });
}

void Server::stop() {
  resume();
  boost::unique_lock<boost::mutex> lk(pImpl->rpcing);
  pImpl->cv.wait(lk, [this] { return !pImpl->numRpcRemaining; });
  if (pImpl->server)
    pImpl->server->Shutdown();
  pImpl->runningThread.join();
}

} // namespace rpc
} // namespace raft
} // namespace quintet