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
    std::size_t replicateNum = 1;
  };

  std::unordered_map<ServerId, std::unique_ptr<FollowerNode>> followerNodes;
  boost::shared_mutex logStatesM;
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
  return pImpl->defaultRPCAppendEntries(std::move(message),
                                        ServerIdentityNo::Leader);
}

Reply IdentityLeader::RPCRequestVote(RequestVoteMessage message) {
  return pImpl->defaultRPCRequestVote(std::move(message),
                                      ServerIdentityNo::Leader);
}

AddLogReply IdentityLeader::RPCAddLog(AddLogMessage message) {
  return pImpl->addLog(message);
}

} // namespace quintet

/* ---------------------- AddLog -------------------------------------------- */

namespace quintet {

AddLogReply IdentityLeader::Impl::addLog(const AddLogMessage &msg) {
  BOOST_LOG(service.logger)
    << "get RPCAddLog from " << msg.srvId.toString();
  LogEntry log;
  log.term = state.get_currentTerm();
  log.prmIdx = msg.prmIdx;
  log.srvId = msg.srvId;
  log.args = msg.args;
  log.opName = msg.opName;
  boost::unique_lock<boost::shared_mutex> logEntriesLk(state.entriesM);
  state.entries.emplace_back(std::move(log));
  logEntriesLk.unlock();
  boost::lock_guard<boost::shared_mutex> logStatesLk(logStatesM);
  logStates.emplace_back(std::make_unique<LogState>());
//  service.heartBeatController.restart(); TODO
  return { true, info.local };
}

} // namespace quintet

/* -------------------- AppendEntries --------------------------------------- */

namespace quintet {

Reply IdentityLeader::Impl::sendAppendEntries(rpc::RpcClient &client,
                                              AppendEntriesMessage msg,
                                              const ServerId & logSrv) {
  return client.callRpcAppendEntries(rpc::makeClientContext(100), msg);
}

namespace {

AppendEntriesMessage createAppendEntriesMessage(const ServerState & state,
                                                const ServerInfo & info,
                                                Index start) {

  AppendEntriesMessage msg;
  msg.term = state.get_currentTerm();
  msg.commitIdx = state.get_commitIdx();
  msg.leaderId = info.local;
  assert(start > 0);
  msg.prevLogIdx = start - 1;

  boost::shared_lock<boost::shared_mutex> logEntriesLk(state.entriesM);
  msg.prevLogTerm = state.entries.at(msg.prevLogIdx).term;
  msg.logEntries = std::vector<LogEntry>(
      state.entries.begin() + start, state.entries.end());
  return msg;
}

}

void IdentityLeader::Impl::tryAppendEntries(const ServerId & target) {
  boost::this_thread::disable_interruption di;
  int randId = intRand(100, 999);
  BOOST_LOG(service.logger)
    << "{" << randId << "} tryAppendEntries(" << target.toString() << ")";


  rpc::RpcClient client(grpc::CreateChannel(
    target.toString(), grpc::InsecureChannelCredentials()));

  auto & followerNode = followerNodes.at(target);
  boost::unique_lock<FollowerNode> followerNodeLk(*followerNode);
  auto oldMatchIdx = followerNode->matchIdx;

  std::size_t retryTimes = 0;
  while (true) {
    try {
      boost::this_thread::restore_interruption ri(di);
      boost::this_thread::interruption_point();
    } catch (boost::thread_interrupted &) {
      BOOST_LOG(service.logger) << "{" << randId << "} interrupted";
      return;
    }
    auto msg = createAppendEntriesMessage(state, info, followerNode->nextIdx);
    debugContext.beforeSendRpcAppendEntries(target, msg);
    Reply res;
    try {
      boost::this_thread::restore_interruption ri(di);
      BOOST_LOG(service.logger)
        << "{" << randId << "} sending... prevLogIdx = "
        << msg.prevLogIdx << ", prevLogTerm = " << msg.prevLogTerm
        << ", lastLogIdx = " << msg.prevLogIdx + msg.logEntries.size();
      res = sendAppendEntries(client, msg, target);
      BOOST_LOG(service.logger) << "{" << randId << "} replied";
    } catch (rpc::RpcError & e) {
      ++retryTimes;
      if (retryTimes > 2) {
        BOOST_LOG(service.logger) << "{" << randId << "} failed. RpcError";
        return;
      } else {
        BOOST_LOG(service.logger)
          << "{" << randId << "} retry. RpcError: "
          << e.what();
      }
      continue;
    }

    BOOST_LOG(service.logger)
      << "{" << randId << "} result: term = " << res.first
      << ", flag = " << res.second;
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

  BOOST_LOG(service.logger) << "{" << randId << "}"
    << "matchIdx: " << oldMatchIdx << " -> " << followerNode->matchIdx;
  followerNodeLk.unlock();

  // Succeed.
  boost::shared_lock<boost::shared_mutex> logStatesLk(logStatesM);
  BOOST_LOG(service.logger)
    << "{" << randId << "} logStates.size() = " << logStates.size();
  for (std::size_t i = oldMatchIdx + 1; i < logStates.size(); ++i) {
    BOOST_LOG(service.logger) << "{" << randId << "} 2";
    boost::lock_guard<LogState> logLk(*logStates[i]);
    LogState & logState = *logStates[i];
    ++logState.replicateNum;
    BOOST_LOG(service.logger)
      << "{" << randId << "} logStates[" << i << "].replicateNum = "
      << logState.replicateNum;
    if (logState.replicateNum > info.srvList.size() / 2) {
      boost::unique_lock<boost::shared_mutex> commitIdxLk(state.commitIdxM);
      Index newCommitIdx = std::max(state.commitIdx, i);
      if (state.commitIdx < newCommitIdx) {
        BOOST_LOG(service.logger)
          << "commitIdx: " << state.commitIdx << " -> " << newCommitIdx;
        state.commitIdx = newCommitIdx;
      }
    }
  }

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
  logStates.resize(state.entries.size());
  for (auto & log : logStates)
    log = std::make_unique<LogState>();
}

void IdentityLeader::Impl::init() {
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
  service.heartBeatController.bind(info.electionTimeout / 2, heartBeat);
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