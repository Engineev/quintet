#ifndef QUINTET_IDENTITY_BASE_H
#define QUINTET_IDENTITY_BASE_H

#include "common.h"
#include "raft/raft_common.h"

namespace quintet {
namespace raft {

class IdentityBase {
public:
  virtual ~IdentityBase() = default;

//  virtual std::pair<Term /*current term*/, bool /*success*/>
//  RPCAppendEntries(AppendEntriesMessage message) = 0;

//  virtual std::pair<Term /*current term*/, bool /*vote granted*/>
//  RPCRequestVote(RequestVoteMessage message) = 0;

//  virtual AddLogReply RPCAddLog(AddLogMessage msg) = 0;

  virtual void leave() = 0;

  /// \brief To be invoked right after the transformation to initialize
  ///        [non-blocking]
  virtual void init() = 0;
};

} // namespace raft
} // namespace quintet

#endif //QUINTET_IDENTITY_BASE_H
