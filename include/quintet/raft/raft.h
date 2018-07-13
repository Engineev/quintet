#ifndef QUINTET_RAFT_H
#define QUINTET_RAFT_H

#include <memory>
#include <functional>

#include "common.h"
#include "raft_common.h"
#include "server_info.h"
#include "debug_context.h"

namespace quintet {
namespace raft {

struct State;

class Raft {
public:
  explicit Raft(const ServerInfo & info);
  ~Raft();

  void BindApply(std::function<void(BasicLogEntry)> apply);

  void Start();

  void Shutdown();

  std::pair<bool, ServerId> AddLog(BasicLogEntry entry);

#ifdef UNIT_TEST
  void setDebugContext(const DebugContext & ctx);

  const ServerInfo & getInfo() const;

  Term getCurrentTerm() const;

//  const State & getState() const;

  // Caution! If the heartbeat is enabled, the invocation will shutdown
  // the original heartbeat
//  void forceHeartBeat();

#endif

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;

}; // class Raft

} // namespace raft
} // namespace quintet

#endif //QUINTET_RAFT_H
