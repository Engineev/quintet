#ifndef QUINTET_RAFT_H
#define QUINTET_RAFT_H

#include <functional>
#include <string>
#include <memory>
#include <utility>
#include <raft/service/rpc/RpcDefs.h>

#include "QuintetDefs.h"
#include "RaftDefs.h"

namespace quintet {

struct ServerInfo; // forward declaration

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
  void setBeforeTransform(std::function<ServerIdentityNo(ServerIdentityNo, ServerIdentityNo)> f);

  void setAfterTransform(std::function<void(ServerIdentityNo, ServerIdentityNo)> f);

  const ServerInfo & getInfo() const;

  Term getCurrentTerm() const;

  void setRpcLatency(std::uint64_t lb, std::uint64_t ub);
#endif

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;

}; // class Raft

class RaftDebugContext {
  using No = ServerIdentityNo;
public:
  void setBeforeTransform(std::function<No(No, No)> f) {
    beforeTransform = std::move(f);
  }
  void setAfterTransform(std::function<void(No, No)> f) {
    afterTransform = std::move(f);
  }

private:
  std::function<No(No, No)> beforeTransform = nullptr;
  std::function<void(No, No)> afterTransform = nullptr;
  std::uint64_t rpcLatencyLb = 0, rpcLatencyUb = 0;
  std::function<void(ServerId, const AppendEntriesMessage &)>
      beforeSendRpcAppendEntries;

}; // class RaftDebugContext

} // namespace quintet

#endif // QUINTET_RAFT_H
