#ifndef QUINTET_RPCSERVICE_H
#define QUINTET_RPCSERVICE_H

#include <memory>
#include <string>

#include "RaftDefs.h"
#include "RpcDefs.h"

namespace quintet {
namespace rpc {

class RpcService {
public:
  RpcService();

  ~RpcService();

  void
  bindAddLog(std::function<quintet::AddLogReply(quintet::AddLogMessage)> f);
  void bindAppendEntries(
      std::function<quintet::Reply(quintet::AppendEntriesMessage)> f);
  void
  bindRequestVote(std::function<quintet::Reply(quintet::RequestVoteMessage)> f);

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
