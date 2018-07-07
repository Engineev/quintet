#pragma once

#include <functional>

#include "./common.h"


namespace quintet {
namespace raft {
namespace rpc {

class Server {
public:
  Server();

  ~Server();

  void bindAppendEntries(std::function<Reply(AppendEntriesMessage)> f);
  void bindRequestVote(std::function<Reply(RequestVoteMessage)> f);

//  void configLogger(const std::string &id);

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
} // namespace raft
} // namespace quintet