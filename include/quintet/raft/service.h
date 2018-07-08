#ifndef QUINTET_SERVICE_H
#define QUINTET_SERVICE_H

#include "./service/apply.h"
#include "./service/identity_transformer.h"

namespace quintet {
namespace raft {

struct Service {
  Apply apply;
  IdentityTransformer identityTransformer;
}; // struct Service

} // namespace raft
} // namespace quintet

#endif //QUINTET_SERVICE_H
