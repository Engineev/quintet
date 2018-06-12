#pragma once

#include "Server.h"
#include "IdentityBase.h"

namespace quintet {

class IdentityFollower : public IdentityBase {
public:
  IdentityFollower(
      ServerState & state, ServerInfo & info, ServerService & service) {}

  ~IdentityFollower() override = default;

  std::pair<Term /*current term*/, bool /*success*/>
  RPCAppendEntries(AppendEntriesMessage message) override { throw ; };

  std::pair<Term /*current term*/, bool /*vote granted*/>
  RPCRequestVote(RequestVoteMessage message) override { throw ; };

  void init() override {}

  void leave() override {}

private:
//  struct Impl;
//  std::unique_ptr<Impl> pImpl;

}; // class IdentityFollower

} // namespace quintet