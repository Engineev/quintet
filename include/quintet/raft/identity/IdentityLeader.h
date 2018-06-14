#pragma once

#include "Server.h"
#include "IdentityBase.h"

namespace quintet {

class IdentityLeader : public IdentityBase {
public:
  IdentityLeader(
      ServerState & state, ServerInfo & info, ServerService & service);

  ~IdentityLeader() override;

  std::pair<Term /*current term*/, bool /*success*/>
  RPCAppendEntries(AppendEntriesMessage message) override;

  std::pair<Term /*current term*/, bool /*vote granted*/>
  RPCRequestVote(RequestVoteMessage message) override;

  void init() override;

  void leave() override;

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;

}; // class IdentityLeader

} // namespace quintet