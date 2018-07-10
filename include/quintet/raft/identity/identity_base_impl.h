#ifndef QUINTET_IDENTITY_BASE_IMPL_H
#define QUINTET_IDENTITY_BASE_IMPL_H

#include "server_info.h"
#include "common.h"
#include "raft/state.h"
#include "raft/service.h"
#include "raft/debug_context.h"
#include "raft/raft_common.h"
#include "raft/rpc/common.h"

namespace quintet {
namespace raft {

struct IdentityBaseImpl {
  IdentityBaseImpl(State &state, const ServerInfo &info, Service &service,
                   const DebugContext &debugContext);

  State &state;
  const ServerInfo &info;
  Service &service;
  const DebugContext &debugContext;

  std::pair<bool, ServerId> defaultAddLog(BasicLogEntry entry);

  Reply defaultRPCRequestVote(RequestVoteMessage msg, IdentityNo identity,
                              int randId = 0);

  Reply defaultRPCAppendEntries(AppendEntriesMessage msg, IdentityNo identity,
                                int randId = 0);

  /// \brief the second bullet of "Rules for Servers, All Servers"
  bool checkRpcTerm(Term term, IdentityNo identity);

  std::shared_ptr<void> applyReadyEntries();
};

} // namespace raft
} // namespace quintet

#endif //QUINTET_IDENTITY_BASE_IMPL_H
