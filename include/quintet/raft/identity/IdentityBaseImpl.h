#ifndef QUINTET_IDENTITYBASEIMPL_H
#define QUINTET_IDENTITYBASEIMPL_H

#include "IdentityBase.h"

namespace quintet {

struct IdentityBaseImpl {
  IdentityBaseImpl(ServerState &state,
                   const ServerInfo &info,
                   ServerService &service,
                   const RaftDebugContext &debugContext);

  ServerState &state;
  const ServerInfo &info;
  ServerService &service;
  const RaftDebugContext &debugContext;

  AddLogReply defaultAddLog(AddLogMessage);

  Reply defaultRPCRequestVote(RequestVoteMessage msg);

  Reply defaultRPCAppendEntries(AppendEntriesMessage msg);

  /// \brief the second bullet of "Rules for Servers, All Servers"
  bool checkRpcTerm(Term term);

  std::shared_ptr<void> applyReadyEntries();
};

} // namespace quintet

#endif //QUINTET_IDENTITYBASEIMPL_H
