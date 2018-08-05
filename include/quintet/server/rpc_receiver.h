#ifndef QUINTET_RPC_RECEIVER_H
#define QUINTET_RPC_RECEIVER_H

#include <memory>

#include "actor_types.h"
#include "common/macro.h"

namespace quintet {

class RpcReceiver : public RpcReceiverActor {
public:
  RpcReceiver();
  ~RpcReceiver();

  void bindMailboxes(RaftActor::Mailbox toRaft);

  // Blocked
  void asyncRun(std::uint16_t port);

  void shutdown();

private:
  GEN_PIMPL_DEF();

}; // class RpcReceiver

} // namespace quintet

#endif //QUINTET_RPC_RECEIVER_H
