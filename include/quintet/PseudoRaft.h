#ifndef QUINTET_PSEUDORAFT_H
#define QUINTET_PSEUDORAFT_H

#include <functional>
#include <queue>

#include <thread>
#include <chrono>

#include "QuintetDefs.h"
#include "RaftDefs.h"
#include "misc/EventQueue.h"

namespace quintet {

class PseudoRaft {
public:
  ServerId Local() const { return id; }

  void BindApply(std::function<void(BasicLogEntry)> apply_) {
    apply = std::move(apply_);
  }

  void AsyncRun() {

  }

  void Stop() {}

  void AddLog(
      const std::string & opName, const std::string & args, PrmIdx prmIdx) {
    q.addEvent([this, opName, args, prmIdx] {
      BasicLogEntry log(opName, args, prmIdx, id);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      apply(std::move(log));
    });
  }

private:
  ServerId id;
  EventQueue q;
  std::function<void(BasicLogEntry)> apply;
};

} // namespace quintet

#endif //QUINTET_PSEUDORAFT_H
