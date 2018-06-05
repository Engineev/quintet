#pragma once

#include "service/IdentityTransformer.h"
#include "service/HeartBeatController.h"
#include "service/rpc/RpcClients.h"

namespace quintet {

struct ServerService {
  HeartBeatController heartBeatController;
  IdentityTransformer identityTransformer;
  rpc::RpcClients     clients;

}; // struct ServerService

} // namespace quintet