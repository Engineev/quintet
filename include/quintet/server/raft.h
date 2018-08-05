#ifndef QUINTET_RAFT_H
#define QUINTET_RAFT_H

#include "actor_types.h"

namespace quintet {

class Raft : public RaftActor {
public:
  Raft();

  ~Raft();

  void start(std::string filename);

  void shutdown();

  void bindMailboxes(
      ConfigActor::Mailbox config,
      IdentityTransformerActor::Mailbox identityTransformer,
      RpcSenderActor::Mailbox rpcSender,
      TimerActor::Mailbox timer);

private:
  GEN_PIMPL_DEF();

}; /* namespace quintet */

} /* namespace quintet */

#endif //QUINTET_RAFT_H
