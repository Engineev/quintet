#include "Raft.h"

#include <array>

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

  // pseudo non-blocking
  void triggerTransformation(ServerIdentityNo target);

  void transform(ServerIdentityNo target);
};

} // namespace quintet

/* ---------------- public member functions --------------------------------- */

namespace quintet {
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

} // namespace quintet

/* --------------------- Test ----------------------------------------------- */

namespace quintet {
} // namespace quintet