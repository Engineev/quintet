#pragma once

#include "Server.h"
#include "IdentityBase.h"

namespace quintet {

class IdentityBogus : public IdentityBase {
public:
  IdentityBogus(
      ServerState &, const ServerInfo &,
      ServerService &, const RaftDebugContext &) {}

  ~IdentityBogus() override = default;

  std::pair<Term /*current term*/, bool /*success*/>
  RPCAppendEntries(AppendEntriesMessage message) override;

  std::pair<Term /*current term*/, bool /*vote granted*/>
  RPCRequestVote(RequestVoteMessage message) override;

  AddLogReply RPCAddLog(AddLogMessage message) override;

  void init() override;

  void leave() override;

//private:
//  struct Impl;
//  std::unique_ptr<Impl> pImpl;

}; // class IdentityBogus

} // namespace quintet