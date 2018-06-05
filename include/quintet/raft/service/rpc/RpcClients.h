#ifndef QUINTET_RPCCLIENTS_H
#define QUINTET_RPCCLIENTS_H

#include <cstddef>
#include <vector>

#include "RaftDefs.h"

namespace quintet {
namespace rpc {

class RpcClients {
public:
  std::pair<Term /*current term*/, bool /*success*/>
  callRpcAppendEntries(ServerId target, Term term, ServerId leaderId,
                       std::size_t prevLogIdx, Term prevLogTerm,
                       std::vector<LogEntry> logEntries, std::size_t commitIdx) {
    throw;
  }

  std::pair<Term /*current term*/, bool /*vote granted*/>
  callRpcRequestVote(ServerId target, Term term, ServerId candidateId,
                     std::size_t lastLogIdx, Term lastLogTerm) {
    throw;
  };

  void stop() {
    throw;
  }

}; // class RpcClients
} // namespace rpc
} // namespace quintet

#endif // QUINTET_RPCCLIENTS_H
