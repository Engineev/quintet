#ifndef QUINTET_RAFTDEBUGCONTEXT_H
#define QUINTET_RAFTDEBUGCONTEXT_H

#include <functional>

#include "RaftDefs.h"

namespace quintet {

class RaftDebugContext {
  using No = ServerIdentityNo;
  friend class Raft;
  friend class IdentityLeader;
  friend class IdentityCandidate;
  friend class IdentityFollower;
public:
  RaftDebugContext() = default;
  GEN_COPY(RaftDebugContext, default);

  RaftDebugContext & setBeforeTransform(std::function<No(No, No)> f) {
    beforeTransform = std::move(f);
    return *this;
  }
  RaftDebugContext & setAfterTransform(std::function<void(No, No)> f) {
    afterTransform = std::move(f);
    return *this;
  }

  RaftDebugContext & setBeforeSendRpcAppendEntries(
      std::function<void(ServerId, const AppendEntriesMessage &)> f) {
    beforeSendRpcAppendEntries = std::move(f);
    return *this;
  }

  RaftDebugContext & setHeartBeatEnabled(bool f) {
    heartBeatEnabled = f;
    return *this;
  }

private:
  std::function<No(No, No)> beforeTransform = [](No from, No to) { return to; };
  std::function<void(No, No)> afterTransform = [](No from, No to) {};

  std::uint64_t rpcLatencyLb = 0, rpcLatencyUb = 0;
  std::function<void(ServerId, const AppendEntriesMessage &)>
    beforeSendRpcAppendEntries = [](ServerId, const AppendEntriesMessage &) {};

  bool heartBeatEnabled = true;
  std::function<void()> heartBeat;
}; // class RaftDebugContext


} // namespace quintet

#endif //QUINTET_RAFTDEBUGCONTEXT_H
