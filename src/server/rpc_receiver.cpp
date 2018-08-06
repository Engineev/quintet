#include <server/rpc_receiver.h>

#include <atomic>
#include <thread>

#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <quintet/server/rpc_receiver.h>

#include "rpc/internal.grpc.pb.h"
#include "server/actor_types.h"

namespace quintet {

class InternalRpcService : public rpc::Internal::Service {
public:
  InternalRpcService() = default;

  void bindMailbox(RaftActor::Mailbox toRaft_);

  grpc::Status AppendEntries(grpc::ServerContext *context,
                             const rpc::AppendEntriesMessage *request,
                             rpc::AppendEntriesReply *response) override;

  grpc::Status RequestVote(grpc::ServerContext *context,
                           const rpc::RequestVoteMessage *request,
                           rpc::RequestVoteReply *response) override;

  void block() { blocked = true; }
  void unblock() { blocked = false; }

private:
  std::atomic<bool> blocked{true};
  RaftActor::Mailbox toRaft;

  void wait() {
    while (blocked)
      std::this_thread::yield();
  }
}; // class InternalService

void InternalRpcService::bindMailbox(RaftActor::Mailbox toRaft_) {
  toRaft = std::move(toRaft_);
}

grpc::Status
InternalRpcService::AppendEntries(grpc::ServerContext *context,
                                  const rpc::AppendEntriesMessage *request,
                                  rpc::AppendEntriesReply *response) {
  wait();

  std::vector<LogEntry> entries;
  for (int i = 0; i < request->entries_size(); ++i) {
    LogEntry entry(request->entries(i).opname(), request->entries(i).args(),
                   request->entries(i).term());
    entries.emplace_back(std::move(entry));
  }
  AppendEntriesMessage msg(request->term(), request->leaderid(),
                           request->prevlogindex(), request->prevlogterm(),
                           std::move(entries), request->leadercommit());
  AppendEntriesReply reply = toRaft.send<tag::AppendEntries>(msg).get();
  response->set_success(reply.get_success());
  response->set_term(reply.get_term());
  return grpc::Status::OK;
}
grpc::Status
InternalRpcService::RequestVote(grpc::ServerContext *context,
                                const rpc::RequestVoteMessage *request,
                                rpc::RequestVoteReply *response) {
  wait();

  RequestVoteMessage msg(request->term(), request->candidateid(),
                         request->lastlogindex(), request->lastlogterm());
  RequestVoteReply reply =
      toRaft.send<tag::RequestVote>(std::move(msg)).get();
  response->set_term(reply.get_term());
  response->set_votegranted(reply.get_voteGranted());
  return grpc::Status::OK;
}

} /* namespace quintet */

namespace quintet {

struct RpcReceiver::Impl {
  InternalRpcService service;
  std::unique_ptr<grpc::Server> server;
  std::thread runningThread;

}; // struct RpcReceiver::Impl

} /* namespace quintet */

namespace quintet {

RpcReceiver::RpcReceiver() : pImpl(std::make_unique<Impl>()) {
  bind<tag::BlockRpcReceiver>(
      std::bind(&InternalRpcService::block, &pImpl->service));
  bind<tag::UnblockRpcReceiver>(
      std::bind(&InternalRpcService::unblock, &pImpl->service));
}
GEN_PIMPL_DTOR(RpcReceiver)

void RpcReceiver::bindMailboxes(RaftActor::Mailbox toRaft) {
  pImpl->service.bindMailbox(std::move(toRaft));
};

void RpcReceiver::asyncRun(std::uint16_t port) {
  std::string addr = "0.0.0.0:" + std::to_string(port);
  grpc::ServerBuilder builder;
  builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
  builder.RegisterService(&pImpl->service);
  pImpl->server = builder.BuildAndStart();
  pImpl->runningThread = std::thread([this] { pImpl->server->Wait(); });
}

void RpcReceiver::shutdown() {
  pImpl->server->Shutdown();
  pImpl->runningThread.join();
}

} /* namespace quintet */
