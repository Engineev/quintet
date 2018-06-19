#include "identity/IdentityLeader.h"

#include <memory>
#include <unordered_map>
#include <vector>

#include <boost/atomic/atomic.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/strict_lock.hpp>

#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>

#include "misc/EventQueue.h"
#include "misc/Rand.h"
#include "misc/Thread.h"
#include "service/log/Common.h"
#include "service/rpc/RpcClient.h"


/* -------------- constructors, destructors and Impl ------------------------ */

namespace quintet {

struct IdentityLeader::Impl : public IdentityBaseImpl {
  Impl(ServerState &state, ServerInfo &info, ServerService &service)
      : IdentityBaseImpl(state, info, service) {
    service.logger.add_attribute(
        "Part", logging::attrs::constant<std::string>("Identity"));
  }

  struct FollowerNode
      : public boost::shared_lockable_adapter<boost::shared_mutex> {
    Index nextIdx = Index(-1);
    Index matchIdx = 0;
    boost::thread appendingThread;
  };
  struct LogState : public boost::shared_lockable_adapter<boost::shared_mutex> {
    std::size_t replicateNum = 0;
  };

  std::unordered_map<ServerId, std::unique_ptr<FollowerNode>> followerNodes;
  std::unordered_map<Index, std::unique_ptr<LogState>> logStates;
  EventQueue applyQueue;

  AddLogReply RPCAddLog(const AddLogMessage & message);

  // append entries
  Reply sendAppendEntries(boost::strict_lock<ServerState> &,
      rpc::RpcClient &client, Index start);

  /// \breif Try indefinitely until succeed or \a nextIdx decrease to
  /// the lowest value.
  void tryAppendEntries(const ServerId & target);

  // commit and reply
  /// \breif Update \a state.commitIdx and apply the newly-committed
  /// log entries asynchronously.
  ///
  /// This function is thread-safe andi t is guaranteed that no log
  /// entry will be applied more than once.
  ///
  /// \param commitIdx the new commitIdx
  void commitAndAsyncApply(boost::strict_lock<ServerState> &, Index commitIdx);

  void init();

}; // struct IdentityLeader::Impl

IdentityLeader::IdentityLeader(ServerState &state, ServerInfo &info,
                               ServerService &service)
    : pImpl(std::make_unique<Impl>(state, info, service)) {}

IdentityLeader::~IdentityLeader() = default;

} // namespace quintet

/* --------------- public member functions & RPC ---------------------------- */

namespace quintet {

void IdentityLeader::init() { pImpl->init(); }

void IdentityLeader::leave() { throw; }

Reply IdentityLeader::RPCAppendEntries(AppendEntriesMessage message) {
  throw;
}

Reply IdentityLeader::RPCRequestVote(RequestVoteMessage message) {
  return pImpl->defaultRPCRequestVote(std::move(message));
}

} // namespace quintet

/* -------------------- AppendEntries --------------------------------------- */

namespace quintet {

// helper functions
namespace {

AppendEntriesMessage createAppendEntriesMessage(
    const ServerState & state, const ServerInfo & info,
    Index start) {
  AppendEntriesMessage msg;
  msg.term = state.get_currentTerm();
  msg.leaderId = info.local;
  msg.prevLogIdx = state.get_entries().size() - 2; // TODO: empty ?
  msg.prevLogTerm = state.get_entries().at(msg.prevLogIdx).term;
  msg.logEntries = std::vector<LogEntry>(
      state.get_entries().begin() + start, state.get_entries().end());
  msg.commitIdx = state.get_commitIdx();
  return msg;
}

auto timeout2Deadline(std::uint64_t ms) {
  return std::chrono::system_clock::now() + std::chrono::milliseconds(ms);
}

} // namespace

Reply IdentityLeader::Impl::sendAppendEntries(
    boost::strict_lock<ServerState> &, rpc::RpcClient &client, Index start) {
  auto msg = createAppendEntriesMessage(state, info, start);
  auto ctx = std::make_shared<grpc::ClientContext>();
  ctx->set_deadline(timeout2Deadline(50));
  return client.callRpcAppendEntries(ctx, msg);
}

void IdentityLeader::Impl::tryAppendEntries(const ServerId & target) {
  rpc::RpcClient client(grpc::CreateChannel(
      target.toString(), grpc::InsecureChannelCredentials()));

  boost::strict_lock<ServerState> serverStateLk(state);
  auto & followerNode = followerNodes.at(target);
  boost::unique_lock<FollowerNode> followerNodeLk(*followerNode);

  while (true) {
    auto res = sendAppendEntries(serverStateLk, client, followerNode->nextIdx);
    // TODO: exception; nextIdx == 0 ?
    if (res.second)
      break;
    if (res.first > state.get_currentTerm()) {
      state.getMutable_currentTerm() = res.first;
      service.identityTransformer.notify(ServerIdentityNo::Follower,
                                         state.get_currentTerm());
      return;
    }
    followerNode->nextIdx--;
    boost::this_thread::interruption_point();
  }

  // Succeed.
  auto oldMatchIdx = followerNode->matchIdx;
  Index newCommitIndex = state.get_commitIdx();
  for (auto iter = logStates.find(oldMatchIdx), end = logStates.end();
      iter != end; ++iter) {
    boost::lock_guard<LogState> logLk(*iter->second);
    LogState & logState = *iter->second;
    ++logState.replicateNum;
    newCommitIndex = std::max(newCommitIndex, iter->first);
  }
  followerNode->matchIdx = state.get_entries().size() - 1;
  followerNode->nextIdx = state.get_entries().size();
  followerNodeLk.unlock();
  commitAndAsyncApply(serverStateLk, newCommitIndex);
}

} // namespace quintet

/* -------------------- Commit and Apply ------------------------------------ */

namespace quintet {

void IdentityLeader::Impl::commitAndAsyncApply(
    boost::strict_lock<ServerState> &, Index commitIdx) {
  if (state.get_commitIdx() >= commitIdx)
    return;
  auto oldCommitIdx = state.get_commitIdx();
  state.getMutable_commitIdx() = commitIdx;

  std::vector<LogEntry> entriesToApply(
      state.get_entries().begin() + oldCommitIdx + 1,
      state.get_entries().begin() + commitIdx + 1);
  applyQueue.addEvent([this, entriesToApply = std::move(entriesToApply)] {
    for (std::size_t i = 0, sz = entriesToApply.size(); i < sz; ++i) {
      service.apply(entriesToApply[i]);
      boost::lock_guard<ServerState> lk(state);
      ++state.getMutable_lastApplied(); // TODO: assert
    }
  });
}

} // namespace quintet

/* -------------------------------------------------------------------------- */

namespace quintet {

void IdentityLeader::Impl::init() {
  service.heartBeatController.bind(info.electionTimeout / 3, [this] {
    for (auto & srv : info.srvList) {
      auto & node = followerNodes.at(srv);
      node->appendingThread.interrupt();
    }
    for (auto & srv : info.srvList) {
      auto & node = followerNodes.at(srv);
      boost::lock_guard<FollowerNode> lk(*node);
      node->appendingThread.join();
      node->appendingThread = boost::thread(
          std::bind(&IdentityLeader::Impl::tryAppendEntries, this, srv));
    }
  });
  service.heartBeatController.start(true, true);
}

AddLogReply IdentityLeader::Impl::RPCAddLog(const AddLogMessage & message) {
  boost::lock_guard<ServerState> lk(state);
  LogEntry log;
  log.term = state.get_currentTerm();
  log.prmIdx = message.prmIdx;
  log.srvId = message.srvId;
  log.args = message.args;
  log.opName = message.opName;
  state.getMutable_entries().emplace_back(std::move(log));
  return {true, NullServerId};
}

} // namespace quintet

