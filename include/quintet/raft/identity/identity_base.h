#ifndef QUINTET_IDENTITY_BASE_H
#define QUINTET_IDENTITY_BASE_H

#include "common.h"
#include "raft/raft_common.h"
#include "raft/rpc/common.h"

namespace quintet {
namespace raft {

class IdentityBase {
public:
  virtual ~IdentityBase() = default;

  virtual Reply RPCAppendEntries(AppendEntriesMessage message, int rid) = 0;

  virtual Reply RPCRequestVote(RequestVoteMessage message, int rid) = 0;

  virtual std::pair<bool, ServerId> AddLog(BasicLogEntry entry) = 0;

  virtual void leave() = 0;

  /// \brief To be invoked right after the transformation to initialize
  ///        [non-blocking]
  virtual void init() = 0;
};

} // namespace raft
} // namespace quintet

#endif //QUINTET_IDENTITY_BASE_H
