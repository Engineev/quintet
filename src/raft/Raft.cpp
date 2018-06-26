#include "Raft.h"

#include <algorithm>
#include <unordered_map>
#include <array>

#include <boost/log/attributes/mutable_constant.hpp>

#include "Identity.h"
#include "Server.h"
#include "misc/EventQueue.h"
#include "misc/Rand.h"
#include "service/log/Common.h"
#include "service/rpc/RpcService.h"

/* -------------- constructors, destructors and Impl ------------------------ */

namespace quintet {

Raft::Raft() : pImpl(std::make_unique<Impl>()) {}

Raft::~Raft() = default;

struct Raft::Impl {
  std::array<std::unique_ptr<IdentityBase>, IdentityNum> identities;
  ServerIdentityNo currentIdentity = ServerIdentityNo::Down;
  logging::attrs::mutable_constant<int> curIdentityAttr{
      (int)ServerIdentityNo::Down};

  ServerState state;
  ServerInfo info;
  ServerService service;
  rpc::RpcService rpc;
  logging::src::logger_mt logger;
  struct RpcThreadNode {
    boost::mutex m;
    boost::thread t;
  };
  EventQueue eventQueue; // to synchronize between transformations and 'AddLog's

  void AddLog(std::string opName, std::string args, PrmIdx idx);

  std::pair<Term /*current term*/, bool /*success*/>
  RPCAppendEntries(AppendEntriesMessage msg);

  std::pair<Term /*current term*/, bool /*vote granted*/>
  RPCRequestVote(RequestVoteMessage msg);

  AddLogReply RPCAddLog(AddLogMessage msg);

  void configure(const std::string &filename);

  void initServerService();

  void initRpcService();

  void asyncRun();

  void stop();

  // pseudo non-blocking
  void triggerTransformation(ServerIdentityNo target);

  void transform(ServerIdentityNo target);

#ifdef UNIT_TEST
  RaftDebugContext debugContext;
  std::function<ServerIdentityNo(ServerIdentityNo from, ServerIdentityNo to)>
      beforeTransform =
          [](ServerIdentityNo from, ServerIdentityNo to) { return to; };
  std::function<void(ServerIdentityNo from, ServerIdentityNo to)>
      afterTransform = [](ServerIdentityNo from, ServerIdentityNo to) {};
  std::atomic<uint64_t> rpcLatencyLb{0}, rpcLatencyUb{0};

  void rpcSleep() {
    std::size_t ub = rpcLatencyUb, lb = rpcLatencyLb;
    if (ub < lb) {
      auto t = lb;
      lb = ub;
      ub = t;
    }
    if (ub > 0) {
      auto time = intRand(lb, ub);
      BOOST_LOG(service.logger) << "RPCLatency = " << time << " ms.";
      boost::this_thread::sleep_for(boost::chrono::milliseconds(time));
    }
  }
#endif
};

} // namespace quintet

/* ---------------- public member functions --------------------------------- */

namespace quintet {

void Raft::Configure(const std::string &filename) {
  pImpl->configure(filename);
}
void Raft::AsyncRun() {
  pImpl->asyncRun();
}
void Raft::Stop() {
  pImpl->stop();
}

ServerId Raft::Local() const {
  return pImpl->info.local;
}

void Raft::BindApply(std::function<void(BasicLogEntry)> f) {
  pImpl->service.apply.bind(std::move(f));
}
} // namespace quintet

/* ---------------------- RPCs ---------------------------------------------- */

namespace quintet {

Reply Raft::Impl::RPCAppendEntries(AppendEntriesMessage msg) {
  BOOST_LOG(logger) << "Get RpcAppendEntries from " << msg.leaderId.toString();
  rpcSleep();
  if (currentIdentity == ServerIdentityNo::Down)
    return {InvalidTerm, false};
  return identities[(int)currentIdentity]->RPCAppendEntries(std::move(msg));
};

Reply Raft::Impl::RPCRequestVote(RequestVoteMessage msg) {
  BOOST_LOG(logger) << "Get RpcRequestVote from " << msg.candidateId.toString();
  rpcSleep();
  if (currentIdentity == ServerIdentityNo::Down)
    return {InvalidTerm, false};
  return identities[(int)currentIdentity]->RPCRequestVote(std::move(msg));
};

AddLogReply Raft::Impl::RPCAddLog(AddLogMessage msg) {
//  BOOST_LOG(logger) << "Get RpcAddLog from " << msg.srvId.toString();
  if (currentIdentity == ServerIdentityNo::Down) return {false, NullServerId};
  return identities[(int)currentIdentity]->RPCAddLog(std::move(msg));
}

} // namespace quintet

/* ------------------ helper functions -------------------------------------- */

namespace quintet {

void Raft::Impl::configure(const std::string &filename) {
  info.load(filename);
  logger.add_attribute(
      "ServerId", logging::attrs::constant<std::string>(info.local.toString()));
  logger.add_attribute("Part", logging::attrs::constant<std::string>("Raft"));
  logger.add_attribute("Identity", curIdentityAttr);
  initServerService();
    identities[(std::size_t)ServerIdentityNo::Follower] =
        std::make_unique<IdentityFollower>(state, info, service, debugContext);
  identities[(std::size_t)ServerIdentityNo::Candidate] =
      std::make_unique<IdentityCandidate>(state, info, service, debugContext);
  identities[(std::size_t)ServerIdentityNo::Leader] =
      std::make_unique<IdentityLeader>(state, info, service, debugContext);
  identities[(std::size_t)ServerIdentityNo::Bogus] =
      std::make_unique<IdentityBogus>(state, info, service, debugContext);
  currentIdentity = ServerIdentityNo::Down;
  initRpcService();
}

void Raft::Impl::initRpcService() {
  rpc.bindRequestVote(
      std::bind(&Raft::Impl::RPCRequestVote, this, std::placeholders::_1));
  rpc.bindAppendEntries(
      std::bind(&Raft::Impl::RPCAppendEntries, this, std::placeholders::_1));
  rpc.bindAddLog(
      std::bind(&Raft::Impl::RPCAddLog, this, std::placeholders::_1));
  rpc.asyncRun(info.local.port);
}

void Raft::Impl::triggerTransformation(ServerIdentityNo target) {
  eventQueue.addEvent([this, target] { transform(target); });
}

void Raft::Impl::transform(ServerIdentityNo target) {
  auto from = currentIdentity;

  auto actualTarget = target;
#ifdef IDENTITY_TEST
  actualTarget = debugContext.beforeTransform(from, target);
#endif
  BOOST_LOG(logger) << "transform from " << IdentityNames[(int)from] << " to "
                    << IdentityNames[(int)target] << " (actually to "
                    << IdentityNames[(int)actualTarget] << ")"
                    << " term = " << state.get_currentTerm();

  rpc.pause();
  if (from != ServerIdentityNo::Down) identities[(std::size_t)from]->leave();

  currentIdentity = actualTarget;
  curIdentityAttr.set((int)currentIdentity);

#ifdef IDENTITY_TEST
  debugContext.afterTransform(from, target);
#endif
  if (actualTarget != ServerIdentityNo::Down)
    identities[(std::size_t)actualTarget]->init();
  rpc.resume();
}

void Raft::Impl::initServerService() { // TODO
  service.configLogger(info.local.toString());
  service.logger.add_attribute("Identity", curIdentityAttr);

  service.identityTransformer.bind(std::bind(&Raft::Impl::triggerTransformation,
                                             this, std::placeholders::_1));

  //  std::vector<ServerId> srvs;
  //  std::copy_if(
  //      info.srvList.begin(), info.srvList.end(), std::back_inserter(srvs),
  //      [local = info.local](const ServerId &id) { return local != id; });
  //  service.clients.createStubs(srvs);
  //  service.clients.asyncRun();
}

void Raft::Impl::asyncRun() {
  BOOST_LOG(logger) << "Raft::asyncRun()";
  triggerTransformation(ServerIdentityNo::Follower);
}

void Raft::Impl::stop() {
  BOOST_LOG(logger) << "Raft::stop()";
  rpc.stop();
  triggerTransformation(ServerIdentityNo::Down);
  eventQueue.stop();
}

void Raft::Impl::AddLog(std::string opName, std::string args, PrmIdx idx) {
  eventQueue.addEvent([&] {

  });
}

} // namespace quintet

/* --------------------- Test ----------------------------------------------- */

namespace quintet {

const ServerInfo &Raft::getInfo() const {
  return pImpl->info;
}

const ServerState &Raft::getState() const {
  return pImpl->state;
}

Term Raft::getCurrentTerm() const {
  return pImpl->state.get_currentTerm();
}

void Raft::setRpcLatency(std::uint64_t lb, std::uint64_t ub) {
  pImpl->rpcLatencyLb = lb;
  pImpl->rpcLatencyUb = ub;
}

void Raft::setDebugContext(const RaftDebugContext &ctx) {
  pImpl->debugContext = ctx;
}

void Raft::forceHeartBeat() {
  pImpl->service.heartBeatController.start(true, false);
}

} // namespace quintet