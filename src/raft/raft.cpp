#include "raft/raft.h"

#include <array>

#include <boost/thread/future.hpp>

#include "raft/state.h"
#include "raft/service.h"
#include "raft/rpc/rpc_server.h"
#include "raft/identity/identity.h"
#include "misc/event_queue.h"
#include "misc/macro.h"

namespace quintet {
namespace raft {

struct Raft::Impl {
  explicit Impl(const ServerInfo & info) : info(info) { init(); }

  std::array<std::unique_ptr<IdentityBase>, IdentityNum> identities;
  IdentityNo curIdentity = IdentityNo::Down;

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

}; // struct Raft::Impl

void Raft::Impl::triggerTransformation(IdentityNo target) {
  eventQueue.addEvent([this, target] { transform(target); });
}

void Raft::Impl::transform(IdentityNo target) {
  auto from = curIdentity;

  auto actualTarget = target;
#ifdef IDENTITY_TEST
  actualTarget = debugContext.beforeTransform(from, target);
#endif

  rpc.pause();
  if (from != IdentityNo::Down)
    identities[(std::size_t)from]->leave();

  curIdentity = actualTarget;
  reinit();

#ifdef IDENTITY_TEST
  debugContext.afterTransform(from, target);
#endif
  if (actualTarget != IdentityNo::Down)
    identities[(std::size_t)actualTarget]->init();
  rpc.resume();
}

void Raft::Impl::reinit() {
  state.votedFor.clear();
  service.identityTransformer.reset();
}

void Raft::Impl::init() {
//  logger.add_attribute(
//      "ServerId", logging::attrs::constant<std::string>(info.local.toString()));
//  logger.add_attribute("Part", logging::attrs::constant<std::string>("Raft"));
//  logger.add_attribute("Identity", curIdentityAttr);
  service.identityTransformer.bind(std::bind(&Raft::Impl::triggerTransformation,
                                             this, std::placeholders::_1));
  throw ; // TODO
//  identities[(std::size_t)IdentityNo::Follower] =
//      std::make_unique<IdentityFollower>(state, info, service, debugContext);
//  identities[(std::size_t)IdentityNo::Candidate] =
//      std::make_unique<IdentityCandidate>(state, info, service, debugContext);
//  identities[(std::size_t)IdentityNo::Leader] =
//      std::make_unique<IdentityLeader>(state, info, service, debugContext);
  curIdentity = IdentityNo::Down;

  rpc.bindRequestVote(
      std::bind(&Raft::Impl::RPCRequestVote, this, std::placeholders::_1));
  rpc.bindAppendEntries(
      std::bind(&Raft::Impl::RPCAppendEntries, this, std::placeholders::_1));
  rpc.asyncRun(info.get_local().get_port());
}

Reply Raft::Impl::RPCAppendEntries(AppendEntriesMessage msg) {
  if (curIdentity == IdentityNo::Down)
    return {InvalidTerm, false};
  return identities[(int)curIdentity]->RPCAppendEntries(std::move(msg));
}

Reply Raft::Impl::RPCRequestVote(RequestVoteMessage msg) {
  if (curIdentity == IdentityNo::Down)
    return {InvalidTerm, false};
  return identities[(int)curIdentity]->RPCRequestVote(std::move(msg));
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