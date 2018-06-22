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
#include "identity/LeaderStateInterface.h"


/* -------------- constructors, destructors and Impl ------------------------ */

namespace quintet {

struct IdentityLeader::Impl : public IdentityBaseImpl {
  Impl(ServerState &state, const ServerInfo &info,
    ServerService &service, const RaftDebugContext & ctx)
    : IdentityBaseImpl(state, info, service, ctx), state(state) {
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

  LeaderStateInterface state;
  std::unordered_map<ServerId, std::unique_ptr<FollowerNode>> followerNodes;
  std::vector<std::unique_ptr<LogState>> logStates;
  EventQueue applyQueue;

  // append entries
  Reply sendAppendEntries(rpc::RpcClient &client, AppendEntriesMessage msg,
                          const ServerId & logSrv);

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
  void commitAndAsyncApply(Index commitIdx);

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

AddLogReply IdentityLeader::RPCAddLog(AddLogMessage message) {
  auto res = pImpl->state.addLog(message);
  pImpl->service.heartBeatController.restart();
  return res;
}

} // namespace quintet

/* -------------------- AppendEntries --------------------------------------- */

namespace quintet {

Reply IdentityLeader::Impl::sendAppendEntries(rpc::RpcClient &client,
                                              AppendEntriesMessage msg,
                                              const ServerId & logSrv) {
  BOOST_LOG(service.logger)
    << "sending RPCAppendEntries to " << logSrv.toString();
  return client.callRpcAppendEntries(rpc::makeClientContext(50), msg);
}

void IdentityLeader::Impl::tryAppendEntries(const ServerId & target) {
  rpc::RpcClient client(grpc::CreateChannel(
    target.toString(), grpc::InsecureChannelCredentials()));

  auto & followerNode = followerNodes.at(target);
  boost::unique_lock<FollowerNode> followerNodeLk(*followerNode);

  std::size_t retryTimes = 0;
  while (true) {
    boost::this_thread::interruption_point();
    auto msg = state.createAppendEntriesMessage(info, followerNode->nextIdx);
    debugContext.beforeSendRpcAppendEntries(target, msg);
    Reply res;
    try {
      res = sendAppendEntries(client, msg, target);
    }
    catch (rpc::RpcError & e) {
      BOOST_LOG(service.logger)
        << "RpcError: " << e.what() << ". [" << retryTimes << "]";
      ++retryTimes;
      if (retryTimes > 2)
        return;
      continue;
    }

    // success
    if (res.second) {
      followerNode->nextIdx += msg.logEntries.size();
      followerNode->matchIdx = followerNode->nextIdx - 1;
      break;
    }

    // greater term detected
    if (state.set_currentTerm(
        [res](Term curTerm) { return res.first > curTerm; },
        res.first)) {
      service.identityTransformer.notify(ServerIdentityNo::Follower,
                                         res.first);
      return;
    }

    // simply failed.
    if (followerNode->nextIdx == 1)
      return;
    followerNode->nextIdx--;
  }

  // Succeed.
  auto oldMatchIdx = followerNode->matchIdx;
  followerNodeLk.unlock();

  Index newCommitIndex = state.get_commitIdx();
  for (auto iter = logStates.begin() + oldMatchIdx, end = logStates.end();
    iter != end; ++iter) {
    boost::lock_guard<LogState> logLk(**iter);
    LogState & logState = **iter;
    ++logState.replicateNum;
    newCommitIndex = std::max(newCommitIndex,
      std::size_t(iter - logStates.begin()));
  }
  commitAndAsyncApply(newCommitIndex);
}

} // namespace quintet

/* -------------------- Commit and Apply ------------------------------------ */

namespace quintet {

void IdentityLeader::Impl::commitAndAsyncApply(Index commitIdx) {
  Index oldCommitIdx;
  if (!state.set_commitIdx([&oldCommitIdx, commitIdx] (Index curCommitIdx) {
    oldCommitIdx = curCommitIdx;
    return curCommitIdx < commitIdx;
  }, commitIdx))
    return;

  auto entriesToApply = state.sliceLogEntries(oldCommitIdx + 1, commitIdx + 1);
  applyQueue.addEvent([this, entriesToApply = std::move(entriesToApply)]{
    for (std::size_t i = 0, sz = entriesToApply.size(); i < sz; ++i) {
      service.apply(entriesToApply[i]);
      state.incLastApplied();
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
  logStates.reserve(state.entriesSize());
  for (auto & log : logStates)
    log = std::make_unique<LogState>();
}

void IdentityLeader::Impl::init() {
  BOOST_LOG(service.logger) << "init";
  reinitStates();
  auto heartBeat = [this] () mutable {
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
  };
  service.heartBeatController.bind(info.electionTimeout / 3, heartBeat);
  applyQueue.start();
  if (debugContext.heartBeatEnabled)
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

} // namespace quintet

