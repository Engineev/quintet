#include "server/rpc_sender.h"

#include <unordered_map>
#include <functional>

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>

#include "rpc/internal.grpc.pb.h"
#include <grpcpp/create_channel.h>

#include "common/macro.h"

namespace quintet {

struct RpcSender::Impl {
  boost::shared_mutex m;
  std::unordered_map<ServerId, std::unique_ptr<rpc::Internal::Stub>> stubs;

  void config(const std::vector<ServerId> & srvList);

  AppendEntriesReply sendAppendEntriesRpc(
      ServerId target, ClientContext ctx, AppendEntriesMessage msg);

  RequestVoteReply sendRequestVoteRpc(
      ServerId target, ClientContext ctx, RequestVoteMessage msg);

}; // struct RpcSender::Impl

void RpcSender::Impl::config(const std::vector<ServerId> &srvList) {
  boost::lock_guard<boost::shared_mutex> lk(m);
  stubs.clear(); // TODO
  for (const auto & srv : srvList)
    stubs.emplace(srv, rpc::Internal::NewStub(grpc::CreateChannel(
        srv.toString(), grpc::InsecureChannelCredentials())));
}
AppendEntriesReply RpcSender::Impl::sendAppendEntriesRpc(
    ServerId target, ClientContext ctx, AppendEntriesMessage msg) {
  assert(ctx.get_timeout());
  grpc::ClientContext gCtx;
  gCtx.set_deadline(std::chrono::system_clock::now() +
      std::chrono::milliseconds(ctx.get_timeout()));

  rpc::AppendEntriesMessage rpcMsg;
  rpcMsg.set_term(msg.get_term());
  rpcMsg.set_leaderid(msg.get_leaderId().toString());
  rpcMsg.set_prevlogindex(msg.get_prevLogIdx());
  rpcMsg.set_prevlogterm(msg.get_prevLogTerm());
  for (const LogEntry & entry : msg.get_logEntries()) {
    rpc::LogEntry rpcEntry;
    rpcEntry.set_opname(entry.get_opName());
    rpcEntry.set_args(entry.get_args());
    rpcEntry.set_term(entry.get_term());
    *rpcMsg.add_entries() = std::move(rpcEntry);
    rpcMsg.set_leadercommit(msg.get_commitIdx());
  }

  boost::shared_lock_guard<boost::shared_mutex> lk(m);
  auto & stub = stubs.at(target);
  rpc::AppendEntriesReply gReply;
  auto status = stub->AppendEntries(&gCtx, std::move(rpcMsg), &gReply);
  if (!status.ok())
    return AppendEntriesReply(false, InvalidTerm); // TODO exception
  return AppendEntriesReply(gReply.success(), gReply.term());
}
RequestVoteReply RpcSender::Impl::sendRequestVoteRpc(
    ServerId target, ClientContext ctx, RequestVoteMessage msg) {
  assert(ctx.get_timeout());
  grpc::ClientContext gCtx;
  gCtx.set_deadline(std::chrono::system_clock::now() +
      std::chrono::milliseconds(ctx.get_timeout()));

  rpc::RequestVoteMessage rpcMsg;
  rpcMsg.set_term(msg.get_term());
  rpcMsg.set_candidateid(msg.get_candidateId().toString());
  rpcMsg.set_lastlogindex(msg.get_lastLogIdx());
  rpcMsg.set_lastlogterm(msg.get_lastLogTerm());

  boost::shared_lock_guard<boost::shared_mutex> lk(m);
  auto & stub = stubs.at(target);
  rpc::RequestVoteReply gReply;
  auto status = stub->RequestVote(&gCtx, std::move(rpcMsg), &gReply);
  if (!status.ok())
    return RequestVoteReply(false, InvalidTerm); // TODO exception
  return RequestVoteReply(gReply.votegranted(), gReply.term());
}

} // namespace quintet

namespace quintet {

RpcSender::RpcSender() : pImpl(std::make_unique<Impl>()) {
  namespace ph = std::placeholders;
  bind<tag::ConfigSender>(std::bind(&Impl::config, &*pImpl, ph::_1));
  bind<tag::SendAppendEntriesRpc>(
      std::bind(&Impl::sendAppendEntriesRpc, &*pImpl, ph::_1, ph::_2, ph::_3));
  bind<tag::SendRequestVoteRpc>(
      std::bind(&Impl::sendRequestVoteRpc, &*pImpl, ph::_1, ph::_2, ph::_3));
}

GEN_PIMPL_DTOR(RpcSender);

} // namespace quintet
