#pragma once

#include "Server.h"
#include "IdentityBase.h"

namespace quintet {

class IdentityCandidate : public IdentityBase {
public:
  IdentityCandidate(
      ServerState & state, const ServerInfo & info,
      ServerService & service, const RaftDebugContext & debugContext);

  ~IdentityCandidate() override;

  std::pair<Term /*current term*/, bool /*success*/>
  RPCAppendEntries(AppendEntriesMessage message) override;

  std::pair<Term /*current term*/, bool /*vote granted*/>
  RPCRequestVote(RequestVoteMessage message) override;

  AddLogReply RPCAddLog(AddLogMessage message) override;

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

} // namespace quintet