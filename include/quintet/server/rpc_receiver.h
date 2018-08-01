#ifndef QUINTET_RPC_RECEIVER_H
#define QUINTET_RPC_RECEIVER_H

#include <memory>

#include "actor_types.h"
#include "common/macro.h"

namespace quintet {

class RpcReceiver {
public:
  RpcReceiver();
  ~RpcReceiver();

  void bindMailboxes(RaftActor::Mailbox toRaft);

  void asyncRun(std::uint16_t port);

private:
  GEN_PIMPL_DEF();

}; // class RpcReceiver

} // namespace quintet

#endif //QUINTET_RPC_RECEIVER_H
