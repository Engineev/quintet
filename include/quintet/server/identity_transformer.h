#ifndef QUINTET_IDENTITY_TRANSFORMER_H
#define QUINTET_IDENTITY_TRANSFORMER_H

#include <functional>

#include "common/macro.h"
#include "actor_types.h"

namespace quintet {

class IdentityTransformer : public IdentityTransformerActor {
public:
  IdentityTransformer();
  ~IdentityTransformer();

  void bindMailboxes(RaftActor::Mailbox raft);

private:
  GEN_PIMPL_DEF();

}; // class IdentityTransformer

} // namespace quintet

#endif //QUINTET_IDENTITY_TRANSFORMER_H
