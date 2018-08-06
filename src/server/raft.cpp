#include <server/raft.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <unordered_map>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/shared_lock_guard.hpp>

#include "common/utils.h"
#include "server/raft_state.h"
#include "server/logger.h"
#include "server/rpc_error.h"

namespace quintet {

struct Raft::Impl {
  ConfigActor::Mailbox toConfig;
  IdentityTransformerActor::Mailbox toIdentityTransformer;
  RpcSenderActor::Mailbox toRpcSender;
  TimerActor::Mailbox toTimer;

  RaftState state;
  ServerInfo info;

  IdentityNo curIdentity = IdentityNo::Down;

  // Some other member variables are declared in the corresponding scopes

  void transform(IdentityNo target);

  std::string getIdentityName(IdentityNo id);

  AppendEntriesReply rpc_appendEntries(AppendEntriesMessage msg);

  RequestVoteReply rpc_requestVote(RequestVoteMessage msg);

  // ------------------------- default -------------------------------------- //

  void sendTriggerTransform(IdentityNo target);

  bool checkRpcTerm(Term term);

  bool LogUpToDate(std::size_t lastLogIdx, Term lastLogTerm);

  std::shared_ptr<void> applyReadyEntries() {
    std::cerr << "unimplemented 'applyReadyEntries()'\n";
    return {};
  }

  AppendEntriesReply default_rpc_appendEntries(AppendEntriesMessage msg);

  RequestVoteReply default_rpc_requestVote(RequestVoteMessage msg);

  // ------------------------- follower ------------------------------------- //

  void foll_init();

  void foll_leave();

  AppendEntriesReply foll_rpc_appendEntries(AppendEntriesMessage msg);

  RequestVoteReply foll_rpc_requestVote(RequestVoteMessage msg);

  // ------------------------- candidate ------------------------------------ //

  std::atomic<std::size_t> votesReceived{0};
  std::vector<std::thread> requesting;

  void requestVotes();

  RequestVoteMessage createRequestVoteMessage() const;

  void cand_init();

  void cand_leave();

  AppendEntriesReply cand_rpc_appendEntries(AppendEntriesMessage msg);

  RequestVoteReply cand_rpc_requestVote(RequestVoteMessage msg);

  // ------------------------- leader    ------------------------------------ //

  struct FollowerState {
    Index nextIdx = 1;
    Index matchIdx = 0;
    std::thread appendingThread;
    boost::shared_mutex m;
  };
  std::unordered_map<ServerId, std::unique_ptr<FollowerState>> followerStates;

  void lead_init();

  void lead_leave();

}; /* struct Raft::Impl */

void Raft::Impl::transform(IdentityNo target) {
  toIdentityTransformer.send<tag::ResetTransformer>().get();
  if (curIdentity == IdentityNo::Down) ;
  else if (curIdentity == IdentityNo::Follower)
    foll_leave();
  else if (curIdentity == IdentityNo::Candidate)
    cand_leave();
  else if (curIdentity == IdentityNo::Leader)
    lead_leave();
  else
    std::terminate();

  Logger::instance().addLog(info.get_local().toString(),
      "Transform from ", getIdentityName(curIdentity),
      " to ", getIdentityName(target));
  curIdentity = target;

  if (target == IdentityNo::Down) ;
  else if (curIdentity == IdentityNo::Follower)
    foll_init();
  else if (target == IdentityNo::Candidate)
    cand_init();
  else if (curIdentity == IdentityNo::Leader)
    lead_init();
  else
    std::terminate();
}

std::string Raft::Impl::getIdentityName(IdentityNo id) {
  if (id == IdentityNo::Follower)
    return "follower";
  if (id == IdentityNo::Candidate)
    return "candidate";
  if (id == IdentityNo::Leader)
    return "leader";
  if (id == IdentityNo::Down)
    return "down";
  throw;
}

AppendEntriesReply Raft::Impl::rpc_appendEntries(AppendEntriesMessage msg) {
  // clang-format off
  Logger::instance().addLog(
      info.get_local().toString(), curIdentity, "{", msg.get_debugId(), "}",
      "get AppendEntries RPC from ", msg.get_leaderId().toString());
  // clang-format on
  if (curIdentity == IdentityNo::Follower)
    return foll_rpc_appendEntries(std::move(msg));
  if (curIdentity == IdentityNo::Candidate)
    return cand_rpc_appendEntries(std::move(msg));
  std::terminate();
}

RequestVoteReply Raft::Impl::rpc_requestVote(RequestVoteMessage msg) {
  // clang-format off
  Logger::instance().addLog(
      info.get_local().toString(), curIdentity, "{", msg.get_debugId(), "}",
      "get RequestVote RPC from ", msg.get_candidateId().toString());
  // clang-format on
  if (curIdentity == IdentityNo::Follower)
    return foll_rpc_requestVote(std::move(msg));
  if (curIdentity == IdentityNo::Candidate)
    return cand_rpc_requestVote(std::move(msg));
  std::terminate();
}

// ------------------------- default ---------------------------------------- //

void Raft::Impl::sendTriggerTransform(IdentityNo target) {
  toIdentityTransformer.send<tag::TriggerTransform>(target).get();
}

bool Raft::Impl::checkRpcTerm(Term term) {
  boost::lock_guard<boost::shared_mutex> lk(state.currentTermM);
  if (state.currentTerm > term) {
    //    BOOST_LOG(service.logger)
    //        << "currentTerm: " << state.currentTerm << " -> " << term;
    state.currentTerm = term;
    if (curIdentity != IdentityNo::Follower)
      sendTriggerTransform(IdentityNo::Follower);
    return true;
  }
  //  if (state.currentTerm == term && identity == ServerIdentityNo::Candidate)
  //  {
  //    service.identityTransformer.notify(ServerIdentityNo::Follower, term);
  //    return true;
  //  }
  return false;
}

bool Raft::Impl::LogUpToDate(std::size_t lastLogIdx, Term lastLogTerm) {
  assert(!state.entries.empty());
  if (state.entries.back().get_term() < lastLogTerm)
    return true;
  return state.entries.back().get_term() == lastLogTerm &&
         state.entries.size() - 1 <= lastLogIdx;
}

RequestVoteReply Raft::Impl::default_rpc_requestVote(RequestVoteMessage msg) {
  using std::to_string;
  checkRpcTerm(msg.get_term());
  auto defer = applyReadyEntries();

  // 1.
  {
    std::lock_guard<boost::shared_mutex> currentTermLk(state.currentTermM);
    if (msg.get_term() < state.currentTerm) {
      return {state.currentTerm, false};
    }
  }
  // 2.
  {
    std::lock(state.currentTermM, state.voteForM);
    std::lock_guard<boost::shared_mutex> currentTermLk(state.currentTermM,
                                                       std::adopt_lock);
    std::lock_guard<boost::shared_mutex> voteForLk(state.voteForM,
                                                   std::adopt_lock);
    if (msg.get_term() > state.currentTerm) {
      state.votedFor = msg.get_candidateId();
      state.currentTerm = msg.get_term();
      if (curIdentity != IdentityNo::Follower) {
        sendTriggerTransform(IdentityNo::Follower);
      } else {
        toTimer.send<tag::TimerRestart>().get();
      }
      return {state.currentTerm, true};
    }
  }

  {
    std::unique_lock<boost::shared_mutex> voteForLk(state.voteForM,
                                                    std::defer_lock);
    boost::shared_lock<boost::shared_mutex> entriesLk(state.entriesM,
                                                      boost::defer_lock);
    boost::shared_lock<boost::shared_mutex> currentTermLk(state.currentTermM,
                                                          boost::defer_lock);
    std::lock(voteForLk, entriesLk, currentTermLk);
    if ((state.votedFor.valid() || state.votedFor == msg.get_candidateId()) &&
        LogUpToDate(msg.get_lastLogIdx(), msg.get_lastLogTerm())) {
      state.votedFor = msg.get_candidateId();
      if (curIdentity == IdentityNo::Follower)
        toTimer.send<tag::TimerRestart>().get();
      return {state.currentTerm, true};
    }
  }
  return {state.currentTerm, false};
}

AppendEntriesReply
Raft::Impl::default_rpc_appendEntries(AppendEntriesMessage msg) {
  checkRpcTerm(msg.get_term());
  auto defer = applyReadyEntries();
  auto curTerm = state.syncGet_currentTerm();
  { // 1.
    if (msg.get_term() < curTerm) {
      return {curTerm, false};
    }
  }
  { // 2. up to date ?
    boost::shared_lock<boost::shared_mutex> entriesLk(state.entriesM);
    if (state.entries.size() <= msg.get_prevLogIdx() ||
        state.entries.at(msg.get_prevLogIdx()).get_term() !=
            msg.get_prevLogTerm()) {
      return {curTerm, false};
    }
  }
  { // 3. & 4.
    boost::lock_guard<boost::shared_mutex> entriesLk(state.entriesM);
    Index idxOffset = 0;
    while (idxOffset < msg.get_logEntries().size() &&
           msg.get_prevLogIdx() + idxOffset + 1 < state.entries.size() &&
           state.entries[msg.get_prevLogIdx() + idxOffset + 1].get_term() ==
               msg.get_logEntries()[idxOffset].get_term())
      ++idxOffset;
    if (idxOffset != msg.get_logEntries().size()) { // Conflict does exist
      state.entries.resize(msg.get_prevLogIdx() + idxOffset + 1);
      std::copy(msg.get_logEntries().cbegin() + idxOffset,
                msg.get_logEntries().cend(), std::back_inserter(state.entries));
    } else {
      //      BOOST_LOG(service.logger)
      //          << "{" << randId << "} Nothing to be appended";
    }
  }
  { // 5.
    boost::shared_lock<boost::shared_mutex> entriesLk(state.entriesM,
                                                      boost::defer_lock);
    boost::unique_lock<boost::shared_mutex> commitIdx(state.commitIdxM,
                                                      boost::defer_lock);
    if (msg.get_commitIdx() > state.commitIdx) {
      assert(!state.entries.empty());
      Index newCommitIdx =
          std::min(msg.get_commitIdx(), state.entries.size() - 1);
      if (state.commitIdx != newCommitIdx)
        state.commitIdx = newCommitIdx;
    }
  }
  return {curTerm, true};
}

// ------------------------- follower --------------------------------------- //

void Raft::Impl::foll_init() {
  auto electionTimeout = intRand<std::uint64_t>(
      info.get_electionTimeout(), info.get_electionTimeout() * 2);
  toTimer.send<tag::TimerBind>(electionTimeout, [this] {
    Logger::instance().addLog(info.get_local().toString(), IdentityNo::Follower,
        "ElectionTimeout exceeded.");
    sendTriggerTransform(IdentityNo::Candidate);
  }).get();
  toTimer.send<tag::TimerStart>(false, false).get();
}

void Raft::Impl::foll_leave() {
  toTimer.send<tag::TimerStop>().get();
}

AppendEntriesReply
Raft::Impl::foll_rpc_appendEntries(AppendEntriesMessage msg) {
  auto curTerm = state.syncGet_currentTerm();
  if (msg.get_term() < curTerm)
    return {curTerm, false};

  toTimer.send<tag::TimerRestart>().get();
  return default_rpc_appendEntries(std::move(msg));
}

RequestVoteReply Raft::Impl::foll_rpc_requestVote(RequestVoteMessage msg) {
  return default_rpc_requestVote(std::move(msg));
}

// ------------------------- candidate -------------------------------------- //

RequestVoteMessage Raft::Impl::createRequestVoteMessage() const {
  Term term = state.syncGet_currentTerm();
  ServerId candidateId = info.get_local();
  boost::shared_lock_guard<boost::shared_mutex> lk(state.entriesM);
  Index lastLogIdx = state.entries.size() - 1;
  Term lastLogTerm = state.entries.back().get_term();
  return {term, candidateId, lastLogIdx, lastLogTerm};
}

void Raft::Impl::requestVotes() {
  requesting.reserve(info.get_srvList().size() - 1);
  ClientContext ctx;
  ctx.getMutable_timeout() = 50;

  for (const ServerId &srv : info.get_srvList()) {
    if (srv == info.get_local())
      continue;
    requesting.emplace_back(std::thread([this, srv, ctx]() mutable {
      const auto LocalStr = info.get_local().toString();

      RequestVoteMessage msg = createRequestVoteMessage();
      // clang-format off
      Logger::instance().addLog(
          LocalStr, IdentityNo::Candidate, "{", msg.get_debugId(), "} ",
          "Sending RequestVote RPC to ", srv.toString());
      // clang-format on
      RequestVoteReply reply;
      try {
        reply = toRpcSender.send<tag::SendRequestVoteRpc>(srv, ctx, msg).get();
      } catch (RpcError & e) {
        Logger::instance().addLog(
            LocalStr, IdentityNo::Candidate, "{", msg.get_debugId(), "} ",
            e.what());
        return;
      }
      if (reply.get_term() != msg.get_term() || !reply.get_voteGranted())
        return;
      votesReceived += reply.get_voteGranted();
      if (votesReceived > info.get_srvList().size() / 2)
        sendTriggerTransform(IdentityNo::Leader);
    }));
  }
}

void Raft::Impl::cand_init() {
  state.currentTerm++;
  state.votedFor = info.get_local();
  votesReceived = 1;
  if (votesReceived > info.get_srvList().size() / 2) {
    toIdentityTransformer.send<tag::TriggerTransform>(IdentityNo::Leader).get();
  }

  auto electionTimeout = intRand<std::uint64_t>(info.get_electionTimeout(),
                                                info.get_electionTimeout() * 2);
  requestVotes();

  // clang-format off
  toTimer.send<tag::TimerBind>(electionTimeout, [this] {
    sendTriggerTransform(IdentityNo::Candidate);
  }).get();
  // clang-format on
  toTimer.send<tag::TimerStart>(false, false).get();
}

void Raft::Impl::cand_leave() {
  toTimer.send<tag::TimerStop>().get();
  for (auto &t : requesting)
    t.join();
  requesting.clear();
}

AppendEntriesReply
Raft::Impl::cand_rpc_appendEntries(AppendEntriesMessage msg) {
  Term curTerm = state.syncGet_currentTerm();
  if (msg.get_term() == curTerm)
    sendTriggerTransform(IdentityNo::Follower);
  return default_rpc_appendEntries(std::move(msg));
}

RequestVoteReply Raft::Impl::cand_rpc_requestVote(RequestVoteMessage msg) {
  return default_rpc_requestVote(std::move(msg));
}
// ------------------------- leader ----------------------------------------- //

void Raft::Impl::lead_init() {
  // re-init the states
  for (auto & srv : info.get_srvList()) {
    if (srv == info.get_local())
      continue;
    followerStates.emplace(srv, std::make_unique<FollowerState>());
  }

  auto heartBeat = [this] () mutable {
    for (auto &srv : info.get_srvList()) {
      if (srv == info.get_local())
        continue;
      FollowerState &node = *followerStates.at(srv);
      if (node.appendingThread.joinable())
        node.appendingThread.join();
      std::lock_guard<boost::shared_mutex> lk(node.m);
      node.appendingThread = std::thread([this, srv] {
        // TODO
        ClientContext ctx;
        ctx.getMutable_timeout() = 50;
        AppendEntriesMessage msg(
            state.syncGet_currentTerm(), info.get_local(), 0, 0, {}, 0);
        toRpcSender.send<tag::SendAppendEntriesRpc>(srv, ctx, msg).get();
      });
    }
  };
  toTimer.send<tag::TimerBind>(info.get_electionTimeout() / 2, heartBeat).get();
  toTimer.send<tag::TimerStart>(true, true).get();
}

void Raft::Impl::lead_leave() {
  toTimer.send<tag::TimerStop>().get();
//    for (auto & item : followerStates)
//      item.second->appendingThread.interrupt();
  for (auto & item : followerStates)
    item.second->appendingThread.join();
//    pImpl->service.apply.wait();
  followerStates.clear();
//    pImpl->logStates.clear();
}

} /* namespace quintet */

namespace quintet {

Raft::Raft() : pImpl(std::make_unique<Impl>()) {
  namespace ph = std::placeholders;
  bind<tag::TransformIdentity>(std::bind(&Impl::transform, &*pImpl, ph::_1));
  bind<tag::AppendEntries>(
      std::bind(&Impl::rpc_appendEntries, &*pImpl, ph::_1));
  bind<tag::RequestVote>(
      std::bind(&Impl::rpc_requestVote, &*pImpl, ph::_1));
  // TODO: addLog

}
GEN_PIMPL_DTOR(Raft)

void Raft::start(std::string filename) {
  pImpl->info =
      pImpl->toConfig.send<tag::LoadConfig>(std::move(filename)).get();
  pImpl->transform(IdentityNo::Follower);
}

void Raft::bindMailboxes(ConfigActor::Mailbox config,
                         IdentityTransformerActor::Mailbox identityTransformer,
                         RpcSenderActor::Mailbox rpcSender,
                         TimerActor::Mailbox toTimer) {
  pImpl->toConfig = std::move(config);
  pImpl->toIdentityTransformer = std::move(identityTransformer);
  pImpl->toRpcSender = std::move(rpcSender);
  pImpl->toTimer = std::move(toTimer);
}
void Raft::shutdown() {
  pImpl->transform(IdentityNo::Down);
}


} /* namespace quintet */