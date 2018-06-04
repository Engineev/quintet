#pragma once

#include <memory>
#include <vector>

#include "RaftDefs.h"
#include "Server.h"

namespace quintet {

class IdentityBase {
public:
  virtual ~IdentityBase() = default;

  virtual std::pair<Term /*current term*/, bool /*success*/>
  RPCAppendEntries(Term term, ServerId leaderId, std::size_t prevLogIdx,
                   Term prevLogTerm, std::vector<LogEntry> logEntries,
                   std::size_t commitIdx) = 0;

  virtual std::pair<Term /*current term*/, bool /*vote granted*/>
  RPCRequestVote(Term term, ServerId candidateId, std::size_t lastLogIdx,
                 Term lastLogTerm) = 0;

  virtual void leave() = 0;

  /// \brief To be invoked right after the transformation to initialize
  ///        [non-blocking]
  virtual void init() = 0;

protected:
  struct IdentityBaseImpl {
    IdentityBaseImpl(
        ServerState &state, ServerInfo &info, ServerService &service)
        : state(state), info(info), service(service) {}

    ServerState &state;
    const ServerInfo &info;
    ServerService &service;
  };
};

} // namespace quintet