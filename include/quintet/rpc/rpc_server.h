#ifndef QUINTET_RPC_SERVER_H
#define QUINTET_RPC_SERVER_H

#include <memory>
#include <functional>

#include "common.h"

namespace quintet {
namespace rpc {

class RpcServer {
public:
  RpcServer();
  ~RpcServer();

  void start(Port port);

  void bindAddLog(std::function<std::pair<bool, ServerId>(BasicLogEntry)> f);

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;

}; // class RpcServer

} // namespace rpc
} // namespace quintet

#endif //QUINTET_RPC_SERVER_H
