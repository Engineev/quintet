#ifndef QUINTET_RAFT_H
#define QUINTET_RAFT_H

#include "actor_types.h"

namespace quintet {

class Raft : public RaftActor {
public:
  Raft();
  ~Raft();

  void bindMailboxes(ConfigActor::Mailbox config);

private:
  GEN_PIMPL_DEF();

}; /* namespace quintet */

} /* namespace quintet */

#endif //QUINTET_RAFT_H
