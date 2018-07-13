#pragma once

#include "identity_base.h"
#include "server_info.h"
#include "raft/state.h"
#include "raft/service.h"
#include "raft/debug_context.h"

namespace quintet {
namespace raft {

class IdentityLeader : public IdentityBase {
public:
  IdentityLeader(State &state, const ServerInfo &info, Service &service,
                    const DebugContext &debugContext);

  ~IdentityLeader() override;

  Reply RPCAppendEntries(AppendEntriesMessage message, int rid) override;

  Reply RPCRequestVote(RequestVoteMessage message, int rid) override;

  std::pair<bool, ServerId> AddLog(BasicLogEntry entry) override;

  void init() override;

  void leave() override;

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;

}; // class IdentityLeader

} // namespace raft
} // namespace quintet