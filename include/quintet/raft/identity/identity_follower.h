#ifndef QUINTET_IDENTITY_FOLLOWER_H
#define QUINTET_IDENTITY_FOLLOWER_H

#include "identity_base.h"
#include "server_info.h"
#include "raft/state.h"
#include "raft/service.h"
#include "raft/debug_context.h"

namespace quintet {
namespace raft {

class IdentityFollower : public IdentityBase {
public:
  IdentityFollower(State & state, const ServerInfo & info, Service & service,
                   const DebugContext & context);

  ~IdentityFollower() override;

  Reply RPCAppendEntries(AppendEntriesMessage message) override;

  Reply RPCRequestVote(RequestVoteMessage message) override;

  std::pair<bool, ServerId> AddLog(BasicLogEntry entry) override;

  void leave() override;

  void init() override;

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;

}; // class IdentityFollower

} // namespace raft
} // namespace quintet

#endif //QUINTET_IDENTITY_FOLLOWER_H
