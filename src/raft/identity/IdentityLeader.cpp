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
  Impl(ServerState &state, const ServerInfo &info,
    ServerService &service, const RaftDebugContext & ctx)
    : IdentityBaseImpl(state, info, service, ctx) {
    service.logger.add_attribute(
      "Part", logging::attrs::constant<std::string>("Identity"));
    applyQueue.configLogger(info.local.toString());
  }

  struct FollowerNode
    : public boost::shared_lockable_adapter<boost::shared_mutex> {
    Index nextIdx = 1;
    Index matchIdx = 0;
    boost::thread appendingThread;
  };
  struct LogState : public boost::shared_lockable_adapter<boost::shared_mutex> {
    std::size_t replicateNum = 0;
  };

  std::unordered_map<ServerId, std::unique_ptr<FollowerNode>> followerNodes;
  std::vector<std::unique_ptr<LogState>> logStates;
  EventQueue applyQueue;

  AddLogReply RPCAddLog(const AddLogMessage & message);

  // append entries
  Reply sendAppendEntries(boost::strict_lock<ServerState> &,
    rpc::RpcClient &client, Index start, const ServerId & logSrv);

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

  void reinitStates();

  void init();

  void leave();

}; // struct IdentityLeader::Impl

IdentityLeader::IdentityLeader(
  ServerState &state, const ServerInfo &info,
  ServerService &service, const RaftDebugContext & ctx)
  : pImpl(std::make_unique<Impl>(state, info, service, ctx)) {}

IdentityLeader::~IdentityLeader() = default;

} // namespace quintet

/* --------------- public member functions & RPC ---------------------------- */

namespace quintet {

void IdentityLeader::init() { pImpl->init(); }

void IdentityLeader::leave() { pImpl->leave(); }

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
  assert(start > 0);
  msg.prevLogIdx = start - 1;
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
  boost::strict_lock<ServerState> &, rpc::RpcClient &client, Index start,
  const ServerId & logSrv) {
  BOOST_LOG(service.logger)
    << "sending RPCAppendEntries to " << logSrv.toString();
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

  std::size_t retryTimes = 0;
  while (true) {
    boost::this_thread::interruption_point();
    { // injection
      auto msg = createAppendEntriesMessage(state, info, followerNode->nextIdx);
      debugContext.beforeSendRpcAppendEntries(target, msg);
    }
    Reply res;
    try {
      res = sendAppendEntries(serverStateLk, client, followerNode->nextIdx, target);
    }
    catch (rpc::RpcError & e) {
      BOOST_LOG(service.logger)
        << "RpcError: " << e.what() << ". [" << retryTimes << "]";
      ++retryTimes;
      if (retryTimes > 2)
        return;
      continue;
    }
    if (res.second)
      break;
    if (res.first > state.get_currentTerm()) {
      state.getMutable_currentTerm() = res.first;
      service.identityTransformer.notify(ServerIdentityNo::Follower,
        state.get_currentTerm());
      return;
    }
    if (followerNode->nextIdx == 1)
      return;
    followerNode->nextIdx--;
  }

  // Succeed.
  auto oldMatchIdx = followerNode->matchIdx;
  Index newCommitIndex = state.get_commitIdx();
  for (auto iter = logStates.begin() + oldMatchIdx, end = logStates.end();
    iter != end; ++iter) {
    boost::lock_guard<LogState> logLk(**iter);
    LogState & logState = **iter;
    ++logState.replicateNum;
    newCommitIndex = std::max(newCommitIndex,
      std::size_t(iter - logStates.begin()));
  }
  followerNode->matchIdx = state.get_entries().size() - 1;
  followerNode->nextIdx = state.get_entries().size();
  followerNodeLk.unlock();
  commitAndAsyncApply(serverStateLk, newCommitIndex);
}

} // namespace quintet

/* -------------------- Commit and Apply ------------------------------------ */

namespace quintet {

// TODO: Put this function into impl base 
void IdentityLeader::Impl::commitAndAsyncApply(
  boost::strict_lock<ServerState> &, Index commitIdx) {
  if (state.get_commitIdx() >= commitIdx)
    return;
  auto oldCommitIdx = state.get_commitIdx();
  state.getMutable_commitIdx() = commitIdx;

  std::vector<LogEntry> entriesToApply(
    state.get_entries().begin() + oldCommitIdx + 1,
    state.get_entries().begin() + commitIdx + 1);
  applyQueue.addEvent([this, entriesToApply = std::move(entriesToApply)]{
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

void IdentityLeader::Impl::reinitStates() {
  for (auto & srv : info.srvList) {
    if (srv == info.local)
      continue;
    followerNodes.emplace(srv, std::make_unique<FollowerNode>());
  }
  logStates.reserve(state.get_entries().size());
  for (auto & log : logStates)
    log = std::make_unique<LogState>();
}

void IdentityLeader::Impl::init() {
  BOOST_LOG(service.logger) << "init";
  reinitStates();
  service.heartBeatController.bind(info.electionTimeout / 3, [this] {
    for (auto & srv : info.srvList) {
      if (srv == info.local)
        continue;
      auto & node = followerNodes.at(srv);
      node->appendingThread.interrupt();
    }
    for (auto & srv : info.srvList) {
      if (srv == info.local)
        continue;
      auto & node = followerNodes.at(srv);
      node->appendingThread.join();
      boost::lock_guard<FollowerNode> lk(*node);
      node->appendingThread = boost::thread(
        std::bind(&IdentityLeader::Impl::tryAppendEntries, this, srv));
    }
  });
  applyQueue.start();
  service.heartBeatController.start(true, true);
}

void IdentityLeader::Impl::leave() {
  service.heartBeatController.stop();
  for (auto & item : followerNodes)
    item.second->appendingThread.interrupt();
  for (auto & item : followerNodes)
    item.second->appendingThread.join();
  applyQueue.stop();
  followerNodes.clear();
  logStates.clear();
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
  return { true, NullServerId };
}

} // namespace quintet

