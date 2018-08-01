#ifndef QUINTET_RPC_SENDER_H
#define QUINTET_RPC_SENDER_H

#include <memory>

#include "actor_types.h"

namespace quintet {

class RpcSender : public RpcSenderActor {
public:
  RpcSender();
  ~RpcSender();

private:
  GEN_PIMPL_DEF();

}; // class RpcSender

} /* namespace quintet */

#endif //QUINTET_RPC_SENDER_H
