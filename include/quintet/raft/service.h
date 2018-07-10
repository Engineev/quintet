#ifndef QUINTET_SERVICE_H
#define QUINTET_SERVICE_H

#include "./service/apply.h"
#include "./service/identity_transformer.h"
#include "./service/heartbeat_controller.h"

namespace quintet {
namespace raft {

struct Service {
  Apply apply;
  IdentityTransformer identityTransformer;
  HeartbeatController heartbeatController;
}; // struct Service

} // namespace raft
} // namespace quintet

#endif //QUINTET_SERVICE_H
