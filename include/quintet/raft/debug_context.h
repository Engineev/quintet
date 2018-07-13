#ifndef QUINTET_DEBUG_CONTEXT_H
#define QUINTET_DEBUG_CONTEXT_H

#include <functional>

#include <boost/atomic/atomic.hpp>

#include "./raft_common.h"
#include "./rpc/common.h"
#include "misc/macro.h"

namespace quintet {
namespace raft {

class DebugContext {
public:
  GEN_DEFAULT_CTOR_AND_ASSIGNMENT(DebugContext);

  GEN_HANDLES(beforeTrans);
  GEN_HANDLES(afterTrans);
  GEN_HANDLES(rpcLatencyLb);
  GEN_HANDLES(rpcLatencyUb);
  GEN_HANDLES(beforeSendingRpcAppendEntries);

private:
  using No = IdentityNo;
  std::function<No(No, No)> beforeTrans = [](No, No to) { return to; };
  std::function<void(No, No)> afterTrans = [](No, No) {};
  std::function<void(quintet::ServerId, const AppendEntriesMessage &)>
      beforeSendingRpcAppendEntries
      = [](ServerId, const AppendEntriesMessage &) {};
  uint64_t rpcLatencyLb{0}, rpcLatencyUb{0};
}; // class DebugContext


} // namespace raft
} // namespace quintet

#endif //QUINTET_DEBUG_CONTEXT_H
