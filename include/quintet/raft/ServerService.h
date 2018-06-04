#pragma once

#include "service/IdentityTransformer.h"
#include "service/HeartBeatController.h"

namespace quintet {

struct ServerService {
  HeartBeatController heartBeatController;
  IdentityTransformer identityTransformer;

}; // struct ServerService

} // namespace quintet