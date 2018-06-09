#ifndef QUINTET_RPCCLIENTS_H
#define QUINTET_RPCCLIENTS_H

#include <cstddef>
#include <memory>
#include <utility>

#include <boost/thread/future.hpp>

#include "RaftDefs.h"

namespace quintet {
namespace rpc {

class RpcClients {
public:
  RpcClients();

  ~RpcClients();

  std::pair<Term, bool>
  callRpcAppendEntries(ServerId target, Term term, ServerId leaderId,
                       std::size_t prevLogIdx, Term prevLogTerm,
                       std::vector<LogEntry> logEntries, std::size_t commitIdx);

  boost::future<std::pair<Term, bool>>
  asyncCallRpcAppendEntries(ServerId target, Term term, ServerId leaderId,
                            std::size_t prevLogIdx, Term prevLogTerm,
                            std::vector<LogEntry> logEntries,
                            std::size_t commitIdx);

  std::pair<Term, bool> callRpcRequestVote(ServerId target, Term term,
                                           ServerId candidateId,
                                           std::size_t lastLogIdx,
                                           Term lastLogTerm);

  boost::future<std::pair<Term, bool>>
  asyncCallRpcRequestVote(ServerId target, Term term, ServerId candidateId,
                          std::size_t lastLogIdx, Term lastLogTerm);

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
}; // class RpcClients
} // namespace rpc
} // namespace quintet

#endif // QUINTET_RPCCLIENTS_H
