#include "raft/raft.h"

#include <array>

#include <boost/thread/future.hpp>
#include <boost/log/attributes/mutable_constant.hpp>

#include "raft/state.h"
#include "raft/service.h"
#include "raft/identity/identity.h"
#include "raft/rpc/rpc_server.h"
#include "raft/identity/identity.h"
#include "misc/event_queue.h"
#include "misc/macro.h"
#include "misc/rand.h"


namespace quintet {
namespace raft {

struct Raft::Impl {
  explicit Impl(const ServerInfo & info) : info(info) {
    init();
    BOOST_LOG(service.logger)
      << "==============================="
      << " Raft::Raft() "
      << "===============================";
  }

  std::array<std::unique_ptr<IdentityBase>, IdentityNum> identities;
  IdentityNo curIdentity = IdentityNo::Down;
  logging::attrs::mutable_constant<int> curIdentityAttr{(int)IdentityNo::Down};

  EventQueue eventQueue; // to synchronize between transformations and 'AddLog's

  rpc::Server rpc;
  Service service;
  State state;
  const ServerInfo & info;

  Reply RPCAppendEntries(AppendEntriesMessage msg);

  Reply RPCRequestVote(RequestVoteMessage msg);

  void init();

  // invoke this function when transforming
  void reinit();

  void triggerTransformation(IdentityNo target);

  void transform(IdentityNo target);

#ifdef UNIT_TEST
  DebugContext debugContext;

  void rpcSleep() {
    std::size_t ub = debugContext.get_rpcLatencyUb(),
                lb = debugContext.get_rpcLatencyLb();
    if (ub < lb) {
      auto t = lb;
      lb = ub;
      ub = t;
    }
    if (ub > 0) {
      auto time = intRand(lb, ub);
//      BOOST_LOG(service.logger) << "RPCLatency = " << time << " ms.";
      boost::this_thread::sleep_for(boost::chrono::milliseconds(time));
    }
  }
#endif

}; // struct Raft::Impl

void Raft::Impl::triggerTransformation(IdentityNo target) {
  eventQueue.addEvent([this, target] { transform(target); });
}

void Raft::Impl::transform(IdentityNo target) {
  auto from = curIdentity;

  auto actualTarget = target;
  actualTarget = debugContext.get_beforeTrans()(from, target);
  BOOST_LOG(service.logger)
    << "transform from " << IdentityNames[(int)from] << " to "
    << IdentityNames[(int)target] << " (actually to "
    << IdentityNames[(int)actualTarget] << ")"
    << " term = " << state.currentTerm;

  rpc.pause();
  if (from != IdentityNo::Down)
    identities[(std::size_t)from]->leave();

  curIdentity = actualTarget;
  curIdentityAttr.set((int)curIdentity);
  reinit();

  debugContext.get_afterTrans()(from, target);
  if (actualTarget != IdentityNo::Down)
    identities[(std::size_t)actualTarget]->init();
  rpc.resume();
}

void Raft::Impl::reinit() {
  state.votedFor.clear();
  service.identityTransformer.reset();
}

void Raft::Impl::init() {
  service.logger.add_attribute(
      "ServerId", logging::attrs::constant<std::string>(info.get_local().toString()));
  service.logger.add_attribute("Identity", curIdentityAttr);

  service.identityTransformer.bind(std::bind(&Raft::Impl::triggerTransformation,
                                             this, std::placeholders::_1));
  identities[(std::size_t)IdentityNo::Follower] =
      std::make_unique<IdentityFollower>(state, info, service, debugContext);
  identities[(std::size_t)IdentityNo::Candidate] =
      std::make_unique<IdentityCandidate>(state, info, service, debugContext);
  identities[(std::size_t)IdentityNo::Leader] =
      std::make_unique<IdentityLeader>(state, info, service, debugContext);
  curIdentity = IdentityNo::Down;

  rpc.bindRequestVote(
      std::bind(&Raft::Impl::RPCRequestVote, this, std::placeholders::_1));
  rpc.bindAppendEntries(
      std::bind(&Raft::Impl::RPCAppendEntries, this, std::placeholders::_1));
  rpc.asyncRun(info.get_local().get_port());
}

Reply Raft::Impl::RPCAppendEntries(AppendEntriesMessage msg) {
  int rid = intRand(1000, 1999);
  BOOST_LOG(service.logger) << LOG_RID
    << "Get RpcAppendEntries from " << msg.get_leaderId().toString();
  if (curIdentity == IdentityNo::Down)
    return {InvalidTerm, false};
  return identities[(int)curIdentity]->RPCAppendEntries(std::move(msg), rid);
}

Reply Raft::Impl::RPCRequestVote(RequestVoteMessage msg) {
  int rid = intRand(2000, 2999);
  BOOST_LOG(service.logger) << LOG_RID
    << "Get RpcRequestVote from " << msg.get_candidateId().toString();
  if (curIdentity == IdentityNo::Down)
    return {InvalidTerm, false};
  return identities[(int)curIdentity]->RPCRequestVote(std::move(msg), rid);
}

} // namespace raft
} // namespace quintet

namespace quintet {
namespace raft {

Raft::Raft(const ServerInfo &info) : pImpl(std::make_unique<Impl>(info)) {}
Raft::~Raft() = default;

std::pair<bool, ServerId> Raft::AddLog(BasicLogEntry entry) {
  boost::promise<std::pair<bool, ServerId>> prm;
  auto fut = prm.get_future();
  pImpl->eventQueue.addEvent([this, &prm, entry = std::move(entry)] () mutable {
    auto reply = pImpl->identities[(std::size_t)pImpl->curIdentity]->AddLog(
        std::move(entry));
    prm.set_value(reply);
  });
  return fut.get();
}

void Raft::BindApply(std::function<void(BasicLogEntry)> apply) {
  pImpl->service.apply.bind(std::move(apply));
}

void Raft::Start() {
  pImpl->triggerTransformation(IdentityNo::Follower);
}

void Raft::Shutdown() {
  pImpl->rpc.stop();
  pImpl->triggerTransformation(IdentityNo::Down);
  pImpl->eventQueue.stop();
}

} // namespace raft
} // namespace quintet

namespace quintet {
namespace raft {

void Raft::setDebugContext(const DebugContext &ctx) {
  pImpl->debugContext = ctx;
}

const ServerInfo &Raft::getInfo() const {
  return pImpl->info;
}
Term Raft::getCurrentTerm() const {
  return pImpl->state.syncGet_currentTerm();
}

} // namespace raft
} // namespace quintet