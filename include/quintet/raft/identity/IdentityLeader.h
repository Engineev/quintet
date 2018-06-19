#pragma once

#include "Server.h"
#include "IdentityBase.h"

namespace quintet {

class IdentityLeader : public IdentityBase {
public:
  IdentityLeader(
      ServerState & state, ServerInfo & info, ServerService & service);

  ~IdentityLeader() override;

  Reply RPCAppendEntries(AppendEntriesMessage message) override;

  Reply RPCRequestVote(RequestVoteMessage message) override;

  AddLogReply RPCAddLog(AddLogMessage message) override { throw; }

  void init() override;

  void leave() override;

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;

}; // class IdentityLeader

} // namespace quintet