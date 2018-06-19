#ifndef QUINTET_RAFT_H
#define QUINTET_RAFT_H

#include <functional>
#include <string>
#include <memory>
#include <utility>
#include <raft/service/rpc/RpcDefs.h>

#include "QuintetDefs.h"
#include "RaftDefs.h"

#include <atomic>

namespace quintet {

struct ServerInfo; // forward declaration

class RaftDebugContext {
  using No = ServerIdentityNo;
  friend class Raft;
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

private:
  std::function<No(No, No)> beforeTransform = [](No from, No to) { return to; };
  std::function<void(No, No)> afterTransform = [](No from, No to) {};
  std::uint64_t rpcLatencyLb = 0, rpcLatencyUb = 0;
  std::function<void(ServerId, const AppendEntriesMessage &)>
      beforeSendRpcAppendEntries;

}; // class RaftDebugContext

class Raft {
public:
  Raft();

  ~Raft();

  void BindApply(std::function<void(LogEntry)> apply);

  ServerId Local() const;

  void Configure(const std::string & filename);

  void AsyncRun();

  void Stop();

#ifdef UNIT_TEST
  void setDebugContext(const RaftDebugContext & ctx);

  const ServerInfo & getInfo() const;

  Term getCurrentTerm() const;

  [[deprecated("will be merged into RaftDebugContext")]]
  void setRpcLatency(std::uint64_t lb, std::uint64_t ub);
#endif

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;

}; // class Raft


} // namespace quintet

#endif // QUINTET_RAFT_H
