#ifndef QUINTET_RAFT_H
#define QUINTET_RAFT_H

#include <memory>
#include <functional>

#include "common.h"
#include "raft_common.h"
#include "server_info.h"

namespace quintet {
namespace raft {

class Raft {
public:
  explicit Raft(const ServerInfo & info);
  ~Raft();

  void BindApply(std::function<void(BasicLogEntry)> apply);

  void Start();

  void Shutdown();

  std::pair<bool, ServerId> AddLog(BasicLogEntry entry);

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;

}; // class Raft

} // namespace raft
} // namespace quintet

#endif //QUINTET_RAFT_H
