#pragma once

#include <memory>
#include <vector>

#include "RaftDefs.h"
#include "Server.h"
#include "service/rpc/RpcDefs.h"
#include "RaftDebugContext.h"

namespace quintet {

class IdentityBase {
public:
  virtual ~IdentityBase() = default;

  virtual std::pair<Term /*current term*/, bool /*success*/>
  RPCAppendEntries(AppendEntriesMessage message) = 0;

  virtual std::pair<Term /*current term*/, bool /*vote granted*/>
  RPCRequestVote(RequestVoteMessage message) = 0;

  virtual AddLogReply RPCAddLog(AddLogMessage msg) = 0;

  virtual void leave() = 0;

  /// \brief To be invoked right after the transformation to initialize
  ///        [non-blocking]
  virtual void init() = 0;
};

} // namespace quintet