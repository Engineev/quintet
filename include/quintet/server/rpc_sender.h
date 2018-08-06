#ifndef QUINTET_RPC_SENDER_H
#define QUINTET_RPC_SENDER_H

#include <memory>
#include <exception>

#include "actor_types.h"
#include "common/macro.h"
#include "rpc_error.h"

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
