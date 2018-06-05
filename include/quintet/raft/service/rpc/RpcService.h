#ifndef QUINTET_RPCSERVICE_H
#define QUINTET_RPCSERVICE_H

#include <string>
#include <memory>

#include "RaftDefs.h"

namespace quintet {
namespace rpc {

class RpcService{
public:
  RpcService();

  ~RpcService();

  void configLogger(const std::string &id);

  /// \brief Change the port to listen on to the given one.
  /// The original server will be stopped first and the
  /// functors bound previously will become invalid.
  ///
  /// \param port the port to listen on.
  void listen(Port port);

  void async_run(std::size_t worker = 1);

  /// \breif stop the RPC service. All the ongoing RPCs will
  ///        be finished first.
  ///
  /// Currently, please make sure that the RpcService is running
  /// when stop() is invoked.
  void stop();

  void pause() {}

  /// \brief Resume the paused RPC service and notify all the RPCs waiting.
  void resume() {}

private:
//  struct Impl;
//  std::unique_ptr<Impl> pImpl;
}; // class RpcService

} // namespace rpc
} // namespace quintet

#endif // QUINTET_RPCSERVICE_H
