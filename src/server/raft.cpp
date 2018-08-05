#include <server/raft.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <quintet/server/raft.h>

#include "common/utils.h"
#include "server/raft_state.h"
#include "server/logger.h"

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

  // ------------------------- default -------------------------------------- //

  void sendTriggerTransform(IdentityNo target);

  bool checkRpcTerm(Term term);

  bool LogUpToDate(std::size_t lastLogIdx, Term lastLogTerm);

  std::shared_ptr<void> applyReadyEntries() {
    std::cerr << "unimplemented 'applyReadyEntries()'\n";
    return {};
  }

  AppendEntriesReply default_rpc_appendEntries(AppendEntriesMessage msg,
                                               int rid);

  RequestVoteReply default_rpc_requestVote(RequestVoteMessage msg, int rid);

  // ------------------------- follower ------------------------------------- //

  void foll_init();

  void foll_leave();

  AppendEntriesReply foll_rpc_appendEntries(AppendEntriesMessage msg, int rid);

  RequestVoteReply foll_rpc_requestVote(RequestVoteMessage msg, int rid);

  // ------------------------- candidate ------------------------------------ //

  std::atomic<std::size_t> votesReceived{0};
  std::vector<std::thread> requesting;

  void requestVotes();

  RequestVoteMessage createRequestVoteMessage() const;

  void cand_init();

  void cand_leave();

  AppendEntriesReply cand_rpc_appendEntries(AppendEntriesMessage msg, int rid);

  RequestVoteReply cand_rpc_requestVote(RequestVoteMessage msg, int rid);

  // ------------------------- leader    ------------------------------------ //

}; /* struct Raft::Impl */

void Raft::Impl::transform(IdentityNo target) {
  toIdentityTransformer.send<tag::ResetTransformer>().get();
  if (curIdentity == IdentityNo::Down) ;
  else if (curIdentity == IdentityNo::Follower)
    foll_leave();
  else if (curIdentity == IdentityNo::Candidate)
    cand_leave();
  else
    std::cerr << "Unimplemented 'leave()'\n";

  Logger::instance().addLog(info.get_local().toString(),
      "transform from ", getIdentityName(curIdentity),
      " to ", getIdentityName(target));
  curIdentity = target;

  if (target == IdentityNo::Down) ;
  else if (curIdentity == IdentityNo::Follower)
    foll_init();
  else if (target == IdentityNo::Candidate)
    cand_init();
  else
    std::cerr << "Unimplemented 'init()'\n";
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

RequestVoteReply Raft::Impl::default_rpc_requestVote(RequestVoteMessage msg,
                                                     int rid) {
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
Raft::Impl::default_rpc_appendEntries(AppendEntriesMessage msg, int rid) {
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
    sendTriggerTransform(IdentityNo::Candidate);
  }).get();
  toTimer.send<tag::TimerStart>(false, false).get();
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
  RequestVoteMessage msg = createRequestVoteMessage();
  ClientContext ctx;
  ctx.getMutable_timeout() = 50;

  for (const ServerId &srv : info.get_srvList()) {
    if (srv == info.get_local())
      continue;
    requesting.emplace_back(std::thread([this, srv, msg, ctx]() mutable {
      RequestVoteReply reply =
          toRpcSender.send<tag::SendRequestVoteRpc>(srv, ctx, msg).get();
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
    toIdentityTransformer.send<tag::TriggerTransform>(IdentityNo::Leader).get();
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

AppendEntriesReply Raft::Impl::cand_rpc_appendEntries(AppendEntriesMessage msg,
                                                      int rid) {
  Term curTerm = state.syncGet_currentTerm();
  if (msg.get_term() == curTerm)
    sendTriggerTransform(IdentityNo::Follower);
  return default_rpc_appendEntries(std::move(msg), rid);
}

RequestVoteReply Raft::Impl::cand_rpc_requestVote(RequestVoteMessage msg,
                                                  int rid) {
  return default_rpc_requestVote(std::move(msg), rid);
}
void Raft::Impl::foll_leave() {
  toTimer.send<tag::TimerStop>().get();
}

AppendEntriesReply
Raft::Impl::foll_rpc_appendEntries(AppendEntriesMessage msg, int rid) {
  auto curTerm = state.syncGet_currentTerm();
  if (msg.get_term() < curTerm)
    return {curTerm, false};

  toTimer.send<tag::TimerRestart>().get();
  return default_rpc_appendEntries(std::move(msg), rid);
}

RequestVoteReply
Raft::Impl::foll_rpc_requestVote(RequestVoteMessage msg, int rid) {
  return default_rpc_requestVote(std::move(msg), rid);
}


// ------------------------- leader ----------------------------------------- //

} /* namespace quintet */

namespace quintet {

GEN_PIMPL_CTOR(Raft)
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