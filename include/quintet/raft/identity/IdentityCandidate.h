#pragma once

#include "Server.h"
#include "IdentityBase.h"

namespace quintet {

class IdentityCandidate : public IdentityBase {
public:
  IdentityCandidate(
      ServerState & state, ServerInfo & info, ServerService & service);

  ~IdentityCandidate() override;

  std::pair<Term /*current term*/, bool /*success*/>
  RPCAppendEntries(Term term, ServerId leaderId, std::size_t prevLogIdx,
                   Term prevLogTerm, std::vector<LogEntry> logEntries,
                   std::size_t commitIdx) override;

  std::pair<Term /*current term*/, bool /*vote granted*/>
  RPCRequestVote(Term term, ServerId candidateId, std::size_t lastLogIdx,
                 Term lastLogTerm) override;

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