#pragma once

#include "Server.h"
#include "IdentityBase.h"

namespace quintet {

class IdentityFollower : public IdentityBase {
public:
  IdentityFollower(ServerState & state, const ServerInfo & info,
                   ServerService & service, const RaftDebugContext & context);

  ~IdentityFollower() override;

  std::pair<Term /*current term*/, bool /*success*/>
  RPCAppendEntries(AppendEntriesMessage message) override;

  std::pair<Term /*current term*/, bool /*vote granted*/>
  RPCRequestVote(RequestVoteMessage message) override;

  AddLogReply RPCAddLog(AddLogMessage message) override;

  void init() override;

  void leave() override;

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;

}; // class IdentityFollower

} // namespace quintet