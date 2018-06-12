#include "Raft.h"

#include <array>
#include <algorithm>

#include "Server.h"
#include "raft/service/rpc/RpcService.h"
#include "raft/Identity.h"
#include "misc/EventQueue.h"

/* -------------- constructors, destructors and Impl ------------------------ */

namespace quintet {

Raft::Raft() : pImpl(std::make_unique<Impl>()) {}

Raft::~Raft() = default;

struct Raft::Impl {
  std::array<std::unique_ptr<IdentityBase>, 3> identities;
  ServerIdentityNo currentIdentity = ServerIdentityNo::Down;

  ServerState   state;
  ServerInfo    info;
  ServerService service;
  rpc::RpcService rpc;

  EventQueue transformationQueue;


  std::pair<Term /*current term*/, bool /*success*/>
  RPCAppendEntries(AppendEntriesMessage msg);

  std::pair<Term /*current term*/, bool /*vote granted*/>
  RPCRequestVote(RequestVoteMessage msg);

  void configure(const std::string & filename);

  void initServerService();

  void initRpcService();

  void asyncRun();

  void stop();

  // pseudo non-blocking
  void triggerTransformation(ServerIdentityNo target);

  void transform(ServerIdentityNo target);
};

} // namespace quintet

/* ---------------- public member functions --------------------------------- */

namespace quintet {

void Raft::Configure(const std::string &filename) {
  pImpl->configure(filename);
}
void Raft::AsyncRun() { pImpl->asyncRun(); }
void Raft::Stop() { pImpl->stop(); }

} // namespace quintet

/* ---------------------- RPCs ---------------------------------------------- */

namespace quintet {

std::pair<Term /*current term*/, bool /*success*/>
Raft::Impl::RPCAppendEntries(AppendEntriesMessage msg) {
  if (currentIdentity == ServerIdentityNo::Down)
    return {InvalidTerm, false};

  return identities[(int)currentIdentity]->RPCAppendEntries(std::move(msg));
};

std::pair<Term /*current term*/, bool /*vote granted*/>
Raft::Impl::RPCRequestVote(RequestVoteMessage msg) {
  if (currentIdentity == ServerIdentityNo::Down)
    return {InvalidTerm, false};
  return identities[(int)currentIdentity]->RPCRequestVote(std::move(msg));
};

} // namespace quintet

/* ------------------ helper functions -------------------------------------- */

namespace quintet {

void Raft::Impl::configure(const std::string & filename) {
  info.load(filename);
  initServerService();
  identities[(std::size_t)ServerIdentityNo::Follower]
        = std::make_unique<IdentityFollower>(state, info, service);
  identities[(std::size_t)ServerIdentityNo::Candidate]
      = std::make_unique<IdentityCandidate>(state, info, service);
//    pImpl->identities[(std::size_t)ServerIdentityNo::Leader]
//        = std::make_unique<ServerIdentityLeader>(state, info, service);
  currentIdentity = ServerIdentityNo::Down;
  initRpcService();
}

void Raft::Impl::initRpcService() {
  rpc.bindRequestVote(std::bind(&Raft::Impl::RPCRequestVote, this, std::placeholders::_1));
  rpc.bindAppendEntries(std::bind(&Raft::Impl::RPCAppendEntries, this, std::placeholders::_1));
  rpc.asyncRun(info.local.port);
}

void Raft::Impl::triggerTransformation(ServerIdentityNo target) {
  transformationQueue.addEvent([this, target] { transform(target); });
}

void Raft::Impl::transform(ServerIdentityNo target) {
  auto from = currentIdentity;
  rpc.pause();
  if (from != ServerIdentityNo::Down)
    identities[(std::size_t)from]->leave();
  currentIdentity = target;
  if (target != ServerIdentityNo::Down)
    identities[(std::size_t)target]->init();
  rpc.resume();
}

void Raft::Impl::initServerService() { // TODO
  std::vector<ServerId> srvs;
  std::copy_if(info.srvList.begin(), info.srvList.end(), std::back_inserter(srvs),
               [local = info.local] (const ServerId & id) {
                 return local != id;
               });
  service.clients.createStubs(srvs);
}

void Raft::Impl::asyncRun() {
  triggerTransformation(ServerIdentityNo::Follower);
}

void Raft::Impl::stop() {
  rpc.stop();
  triggerTransformation(ServerIdentityNo::Down);
  transformationQueue.stop();
}

} // namespace quintet

/* --------------------- Test ----------------------------------------------- */

namespace quintet {
} // namespace quintet