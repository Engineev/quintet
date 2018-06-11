#include "service/rpc/RpcService.h"

#include <functional>
#include <utility>
#include <vector>

#include <boost/atomic/atomic.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/thread.hpp>

#include "service/rpc/RaftRpc.grpc.pb.h"
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>

/* ---------------------------- RaftRpc ------------------------------------- */

namespace quintet {
namespace rpc {

class RaftRpcImpl final : public RaftRpc::Service {
  using AppendEntriesFunc = std::function<std::pair<Term, bool>(
      Term, ServerId, std::size_t, Term, std::vector<LogEntry>, std::size_t)>;
  using RequestVoteFunc =
      std::function<std::pair<Term, bool>(Term, ServerId, std::size_t, Term)>;

public:
  void bindAppendEntries(AppendEntriesFunc f) { appendEntries = std::move(f); }
  void bindRequestVote(RequestVoteFunc f) { requestVote = std::move(f); }

  grpc::Status AppendEntries(grpc::ServerContext *context,
                             const AppendEntriesMessage *request,
                             PbReply *response) override {
    Term leaderTerm = request->term();
    ServerId leaderId{request->leaderid().addr(),
                      (Port)request->leaderid().port()};
    std::size_t prevLogIdx = request->prevlogidx();
    Term prevLogTerm = request->prevlogterm();
    std::vector<LogEntry> logEntries;
    for (int i = 0; i < request->logentries_size(); ++i) {
      auto &pbEntry = request->logentries(i);
      LogEntry entry{pbEntry.term(), pbEntry.opname(), pbEntry.args(),
                     pbEntry.prmidx()};
      logEntries.emplace_back(entry);
    }
    std::size_t commitIdx = request->commitidx();

    Term currentTerm;
    bool success;
    std::tie(currentTerm, success) =
        appendEntries(leaderTerm, leaderId, prevLogIdx, prevLogTerm,
                      std::move(logEntries), commitIdx);
    response->set_term(currentTerm);
    response->set_ans(success);

    return grpc::Status::OK;
  }

  grpc::Status RequestVote(grpc::ServerContext *context,
                           const RequestVoteMessage *request,
                           PbReply *response) override {
    Term candidateTerm = request->term();
    ServerId candidateId{request->candidateid().addr(),
                         (Port)request->candidateid().port()};
    std::size_t lastLogIdx = request->lastlogidx();
    Term lastLogTerm = request->lastlogterm();

    Term currentTerm;
    bool granted;
    std::tie(currentTerm, granted) =
        requestVote(candidateTerm, candidateId, lastLogIdx, lastLogTerm);
    response->set_term(currentTerm);
    response->set_ans(granted);

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
      std::function<std::pair<Term, bool>(Term, ServerId, std::size_t, Term,
                                          std::vector<LogEntry>, std::size_t)>
          f) {
    service.bindAppendEntries(
        [this, f](Term term, ServerId leaderId, std::size_t prevLogIdx,
                  Term prevLogTerm, std::vector<LogEntry> logEntries,
                  std::size_t commitIdx) {
          auto defer = preRpc();
          return f(term, leaderId, prevLogIdx, prevLogTerm, std::move(logEntries),
            commitIdx);
        });
  }

  void bindRequestVote(
      std::function<std::pair<Term, bool>(Term, ServerId, std::size_t, Term)> f) {
    service.bindRequestVote([this, f](Term term, ServerId candidateId,
                                      std::size_t lastLogIdx,
                                      Term lastLogTerm) {
      auto defer = preRpc();
      return f(term, candidateId, lastLogIdx, lastLogTerm);
    });
  }

  void pause() {
    paused.lock();
    boost::unique_lock<boost::shared_mutex> lk(rpcing);
  }

  void resume() {
    paused.unlock();
  }

  void stop() {
    boost::unique_lock<boost::mutex> lk(stopping);
    modifiedNumRpcRemaining.wait(lk, [this] {
      return !numRpcRemaining;
    });
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
    std::function<std::pair<Term, bool>(Term, ServerId, size_t, Term,
                                        std::vector<LogEntry>, size_t)>
    f) {
  pImpl->bindAppendEntries(std::move(f));
}
void RpcService::bindRequestVote(
    std::function<std::pair<Term, bool>(Term, ServerId, size_t, Term)> f) {
  pImpl->bindRequestVote(std::move(f));
}

} // namespace rpc
} // namespace quintet