#pragma once

#include "identity_base.h"
#include "server_info.h"
#include "raft/state.h"
#include "raft/service.h"
#include "raft/debug_context.h"

namespace quintet {
namespace raft {

class IdentityCandidate : public IdentityBase {
public:
  IdentityCandidate(State &state, const ServerInfo &info, Service &service,
                    const DebugContext &debugContext);

  ~IdentityCandidate() override;

  Reply RPCAppendEntries(AppendEntriesMessage message) override;

  Reply RPCRequestVote(RequestVoteMessage message) override;

  std::pair<bool, ServerId> AddLog(BasicLogEntry entry) override;

  /// \breif See figure 2 of the paper
  ///
  /// 1. Increase the current term.
  /// 2. Vote for self
  /// 3. Reset election timer
  /// 4. Send RequestVote RPCs to the other servers.
  ///    This procedure will not wait for the other servers to reply.
  void init() override;

  /// \brief clean up
  ///
  /// 1. Interrupt all the remaining RequestVote RPCs
  void leave() override;

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;

}; // class IdentityCandidate

} // namespace raft
} // namespace quintet