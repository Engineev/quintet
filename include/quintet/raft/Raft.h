#ifndef QUINTET_RAFT_H
#define QUINTET_RAFT_H

#include <functional>
#include <string>
#include <memory>
#include <utility>
#include <raft/service/rpc/RpcDefs.h>

#include "QuintetDefs.h"
#include "RaftDefs.h"
#include "RaftDebugContext.h"

namespace quintet {

struct ServerInfo; // forward declaration
struct ServerState;

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

  const ServerState & getState() const;

  [[deprecated("will be merged into RaftDebugContext")]]
  void setRpcLatency(std::uint64_t lb, std::uint64_t ub);

  // Caution! If the heartbeat is enabled, the invocation will shutdown
  // the original heartbeat
  void forceHeartBeat();
#endif

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;

}; // class Raft


} // namespace quintet

#endif // QUINTET_RAFT_H
