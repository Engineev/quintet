#include "raft/identity/identity_leader.h"
#include "raft/identity/identity_base_impl.h"

#include <memory>
#include <unordered_map>
#include <vector>

#include <boost/atomic/atomic.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/strict_lock.hpp>
#include <boost/thread/lockable_adapter.hpp>

#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>

#include "misc/event_queue.h"
#include "misc/rand.h"
#include "raft/rpc/rpc_client.h"
#include "rpc/error.h"


/* -------------------------------------------------------------------------- */

namespace quintet {
namespace raft {

struct IdentityLeader::Impl : public IdentityBaseImpl {
  Impl(State &state, const ServerInfo &info, Service &service,
       const DebugContext &ctx)
      : IdentityBaseImpl(state, info, service, ctx) {
//    service.logger.add_attribute(
//        "Part", logging::attrs::constant<std::string>("Identity"));
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

  // append entries
  Reply sendAppendEntries(rpc::Client &client, AppendEntriesMessage msg,
                          const ServerId &logSrv);

  /// \breif Try indefinitely until succeed or \a nextIdx decrease to
  /// the lowest value.
  void tryAppendEntries(const ServerId &target);

  void reinitStates();


}; // struct IdentityLeader::Impl

void IdentityLeader::Impl::reinitStates() {
  for (auto & srv : info.get_srvList()) {
    if (srv == info.get_local())
      continue;
    followerNodes.emplace(srv, std::make_unique<FollowerNode>());
  }
  logStates.resize(state.entries.size());
  for (auto & log : logStates)
    log = std::make_unique<LogState>();
}

Reply IdentityLeader::Impl::sendAppendEntries(rpc::Client &client,
                                              AppendEntriesMessage msg,
                                              const ServerId & logSrv) {
  ClientContext ctx;
  ctx.setTimeout(100);
  return client.callRpcAppendEntries(ctx, msg);
}
namespace {

AppendEntriesMessage createAppendEntriesMessage(const State & state,
                                                const ServerInfo & info,
                                                Index start) {
  Term term = state.syncGet_currentTerm();
  Index commitIdx = state.syncGet_commitIdx();
  auto leaderId = info.get_local();
  assert(start > 0);
  Index prevLogIdx = start - 1;

  boost::shared_lock<boost::shared_mutex> logEntriesLk(state.entriesM);
  Term prevLogTerm = state.entries.at(prevLogIdx).get_term();
  auto logEntries = std::vector<LogEntry>(
      state.entries.begin() + start, state.entries.end());
  return {term, leaderId, prevLogIdx, prevLogTerm, std::move(logEntries), commitIdx};
}

}

void IdentityLeader::Impl::tryAppendEntries(const ServerId & target) {
  boost::this_thread::disable_interruption di;
  int randId = intRand(100, 999);
//  BOOST_LOG(service.logger)
//      << "{" << randId << "} tryAppendEntries(" << target.toString() << ")";

  rpc::Client client(target);

  auto & followerNode = followerNodes.at(target);
  boost::unique_lock<FollowerNode> followerNodeLk(*followerNode);
  auto oldMatchIdx = followerNode->matchIdx;

  std::size_t retryTimes = 0;
  while (true) {
    try {
      boost::this_thread::restore_interruption ri(di);
      boost::this_thread::interruption_point();
    } catch (boost::thread_interrupted &) {
//      BOOST_LOG(service.logger) << "{" << randId << "} interrupted";
      return;
    }
    auto msg = createAppendEntriesMessage(state, info, followerNode->nextIdx);
    debugContext.get_beforeSendingRpcAppendEntries()(target, msg);
    Reply res;
    try {
      boost::this_thread::restore_interruption ri(di);
//      BOOST_LOG(service.logger)
//          << "{" << randId << "} sending... prevLogIdx = "
//          << msg.prevLogIdx << ", prevLogTerm = " << msg.prevLogTerm
//          << ", lastLogIdx = " << msg.prevLogIdx + msg.logEntries.size();
      res = sendAppendEntries(client, msg, target);
//      BOOST_LOG(service.logger) << "{" << randId << "} replied";
    } catch (quintet::rpc::Error & e) {
      ++retryTimes;
      if (retryTimes > 2) {
//        BOOST_LOG(service.logger) << "{" << randId << "} failed. RpcError";
        return;
      } else {
//        BOOST_LOG(service.logger)
//            << "{" << randId << "} retry. RpcError: "
//            << e.what();
      }
      continue;
    }

//    BOOST_LOG(service.logger)
//        << "{" << randId << "} result: term = " << res.first
//        << ", flag = " << res.second;
    // success
    if (res.get_success()) {
      followerNode->nextIdx += msg.get_logEntries().size();
      followerNode->matchIdx = followerNode->nextIdx - 1;
      break;
    }

    // greater term detected
    boost::unique_lock<boost::shared_mutex> curTermLk(state.currentTermM);
    if (res.get_term() > state.currentTerm) {
      state.currentTerm = res.get_term();
      service.identityTransformer.notify(IdentityNo::Follower);
      return;
    }
    curTermLk.unlock();

    // simply failed.
    if (followerNode->nextIdx == 1)
      return;
    followerNode->nextIdx--;
  }

//  BOOST_LOG(service.logger)
//      << "{" << randId << "} "
//      << "matchIdx: " << oldMatchIdx << " -> " << followerNode->matchIdx;
  followerNodeLk.unlock();

  // Succeed.
  boost::shared_lock<boost::shared_mutex> logStatesLk(logStatesM);
//  BOOST_LOG(service.logger)
//      << "{" << randId << "} logStates.size() = " << logStates.size();
  for (std::size_t i = oldMatchIdx + 1; i < logStates.size(); ++i) {
//    BOOST_LOG(service.logger) << "{" << randId << "} 2";
    boost::lock_guard<LogState> logLk(*logStates[i]);
    LogState & logState = *logStates[i];
    ++logState.replicateNum;
//    BOOST_LOG(service.logger)
//        << "{" << randId << "} logStates[" << i << "].replicateNum = "
//        << logState.replicateNum;
    if (logState.replicateNum > info.get_srvList().size() / 2) {
      boost::unique_lock<boost::shared_mutex> commitIdxLk(state.commitIdxM);
      Index newCommitIdx = std::max(state.commitIdx, i);
      if (state.commitIdx < newCommitIdx) {
//        BOOST_LOG(service.logger)
//            << "commitIdx: " << state.commitIdx << " -> " << newCommitIdx;
        state.commitIdx = newCommitIdx;
      }
    }
  }

  applyReadyEntries();
}


} // namespace raft
} // namespace quintet

/* --------------- public member functions & RPC ---------------------------- */

namespace quintet {
namespace raft {

IdentityLeader::IdentityLeader(
    State &state, const ServerInfo &info,
    Service &service, const DebugContext &ctx)
    : pImpl(std::make_unique<Impl>(state, info, service, ctx)) {}

IdentityLeader::~IdentityLeader() = default;

void IdentityLeader::init() {
  pImpl->reinitStates();
  auto heartBeat = [this] () mutable {
    for (auto & srv : pImpl->info.get_srvList()) {
      if (srv == pImpl->info.get_local())
        continue;
      auto & node = pImpl->followerNodes.at(srv);
      node->appendingThread.interrupt();
    }
    for (auto & srv : pImpl->info.get_srvList()) {
      if (srv == pImpl->info.get_local())
        continue;
      auto & node = pImpl->followerNodes.at(srv);
      node->appendingThread.join();
      boost::lock_guard<Impl::FollowerNode> lk(*node);
      node->appendingThread = boost::thread([this, srv] {
        pImpl->tryAppendEntries(srv);
      });
    }
  };
  pImpl->service.heartbeatController.bind(pImpl->info.get_electionTimeout() / 2,
                                          heartBeat);
//  if (pImpl->debugContext.heartBeatEnabled)
  pImpl->service.heartbeatController.start(true, true);
}

void IdentityLeader::leave() {
  pImpl->service.heartbeatController.stop();
  for (auto & item : pImpl->followerNodes)
    item.second->appendingThread.interrupt();
  for (auto & item : pImpl->followerNodes)
    item.second->appendingThread.join();
  pImpl->service.apply.wait();
  pImpl->followerNodes.clear();
  pImpl->logStates.clear();
}

Reply IdentityLeader::RPCAppendEntries(AppendEntriesMessage message, int rid) {
  return pImpl->defaultRPCAppendEntries(std::move(message), IdentityNo::Leader);
}

Reply IdentityLeader::RPCRequestVote(RequestVoteMessage message, int rid) {
  return pImpl->defaultRPCRequestVote(std::move(message), IdentityNo::Leader);
}

std::pair<bool, ServerId> IdentityLeader::AddLog(BasicLogEntry entry) {
//  BOOST_LOG(service.logger)
//      << "get RPCAddLog from " << msg.srvId.toString();
  LogEntry log(entry.get_opName(), entry.get_args(), entry.get_prmIdx(),
               pImpl->state.syncGet_currentTerm());
  boost::unique_lock<boost::shared_mutex> logEntriesLk(pImpl->state.entriesM);
  for (auto riter = pImpl->state.entries.rbegin();
      riter != pImpl->state.entries.rend(); ++riter) {
    // TODO: For now, rand a prmIdx to identify the entry
    if (riter->get_prmIdx() == entry.get_prmIdx())
      return {true, pImpl->info.get_local()};
  }
  pImpl->state.entries.emplace_back(std::move(log));
  logEntriesLk.unlock();
  boost::lock_guard<boost::shared_mutex> logStatesLk(pImpl->logStatesM);
  pImpl->logStates.emplace_back(std::make_unique<Impl::LogState>());
//  service.heartBeatController.restart(); TODO
  return {true, pImpl->info.get_local()};
}

} // namespace raft
} // namespace quintet

