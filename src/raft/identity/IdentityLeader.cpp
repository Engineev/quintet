#include "identity/IdentityLeader.h"
#include "identity/IdentityBaseImpl.h"

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

  AddLogReply addLog(const AddLogMessage & msg);

  // append entries
  Reply sendAppendEntries(rpc::RpcClient &client, AppendEntriesMessage msg,
                          const ServerId & logSrv);

  /// \breif Try indefinitely until succeed or \a nextIdx decrease to
  /// the lowest value.
  void tryAppendEntries(const ServerId & target);

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
  return pImpl->defaultRPCAppendEntries(std::move(message));
}

Reply IdentityLeader::RPCRequestVote(RequestVoteMessage message) {
  return pImpl->defaultRPCRequestVote(std::move(message));
}

AddLogReply IdentityLeader::RPCAddLog(AddLogMessage message) {
  return pImpl->addLog(message);
}

} // namespace quintet

/* ---------------------- AddLog -------------------------------------------- */

namespace quintet {

AddLogReply IdentityLeader::Impl::addLog(const AddLogMessage &msg) {
  boost::unique_lock<boost::shared_mutex> logEntriesLk(state.entriesM);
  LogEntry log;
  log.term = state.get_currentTerm();
  log.prmIdx = msg.prmIdx;
  log.srvId = msg.srvId;
  log.args = msg.args;
  log.opName = msg.opName;
  state.entries.emplace_back(std::move(log));
  service.heartBeatController.restart();
  return { true, NullServerId };
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

namespace {

AppendEntriesMessage createAppendEntriesMessage(const ServerState & state,
                                                const ServerInfo & info,
                                                Index start) {
  boost::shared_lock<boost::shared_mutex>
      commitIdxLk(state.commitIdxM, boost::defer_lock),
      currentTermLk(state.currentTermM, boost::defer_lock),
      logEntriesLk(state.entriesM, boost::defer_lock);
  boost::lock(commitIdxLk, currentTermLk, logEntriesLk);

  AppendEntriesMessage msg;
  msg.term = state.currentTerm;
  msg.leaderId = info.local;
  assert(start > 0);
  msg.prevLogIdx = start - 1;
  msg.prevLogTerm = state.entries.at(msg.prevLogIdx).term;
  msg.logEntries = std::vector<LogEntry>(
      state.entries.begin() + start, state.entries.end());
  msg.commitIdx = state.commitIdx;

  return msg;
}

}

void IdentityLeader::Impl::tryAppendEntries(const ServerId & target) {
  rpc::RpcClient client(grpc::CreateChannel(
    target.toString(), grpc::InsecureChannelCredentials()));

  auto & followerNode = followerNodes.at(target);
  boost::unique_lock<FollowerNode> followerNodeLk(*followerNode);

  std::size_t retryTimes = 0;
  while (true) {
    boost::this_thread::interruption_point();
    auto msg = createAppendEntriesMessage(state, info, followerNode->nextIdx);
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
    boost::unique_lock<boost::shared_mutex> curTermLk(state.currentTermM);
    if (res.first > state.currentTerm) {
      state.currentTerm = res.first;
      service.identityTransformer.notify(ServerIdentityNo::Follower,
                                         res.first);
      return;
    }
    curTermLk.unlock();

    // simply failed.
    if (followerNode->nextIdx == 1)
      return;
    followerNode->nextIdx--;
  }

  // Succeed.
  auto oldMatchIdx = followerNode->matchIdx;
  followerNodeLk.unlock();

  boost::unique_lock<boost::shared_mutex> commitIdxLk(state.commitIdxM);
  for (auto iter = logStates.begin() + oldMatchIdx, end = logStates.end();
    iter != end; ++iter) {
    boost::lock_guard<LogState> logLk(**iter);
    LogState & logState = **iter;
    ++logState.replicateNum;
    state.commitIdx = std::max(state.commitIdx,
        std::size_t(iter - logStates.begin()));
  }
  commitIdxLk.unlock();

  applyReadyEntries();
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
  logStates.reserve(state.entries.size());
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
  if (debugContext.heartBeatEnabled)
    service.heartBeatController.start(true, true);
}

void IdentityLeader::Impl::leave() {
  service.heartBeatController.stop();
  for (auto & item : followerNodes)
    item.second->appendingThread.interrupt();
  for (auto & item : followerNodes)
    item.second->appendingThread.join();
  service.apply.wait();
  followerNodes.clear();
  logStates.clear();
}

} // namespace quintet