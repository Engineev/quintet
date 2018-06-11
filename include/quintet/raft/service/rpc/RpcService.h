#ifndef QUINTET_RPCSERVICE_H
#define QUINTET_RPCSERVICE_H

#include <memory>
#include <string>

#include "RaftDefs.h"

namespace quintet {
namespace rpc {

class RpcService {
public:
  RpcService();

  ~RpcService();

  void bindAppendEntries(
      std::function<std::pair<Term, bool>(Term, ServerId, size_t, Term,
                                          std::vector<LogEntry>, size_t)>
          f);
  void bindRequestVote(
      std::function<std::pair<Term, bool>(Term, ServerId, size_t, Term)> f);

  void configLogger(const std::string &id);

  void asyncRun(Port port);

  /// \brief Graceful shutdown.
  void stop();

  void pause();

  /// \brief Resume the paused RPC service and notify all the RPCs waiting.
  void resume();

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
}; // class RpcService

} // namespace rpc
} // namespace quintet

#endif // QUINTET_RPCSERVICE_H
