#pragma once

#include <functional>
#include <vector>

#include "../raft_common.h"
#include "misc/event_queue.h"

namespace quintet {
namespace raft {

class Apply {
public:
  void bind(std::function<void(quintet::BasicLogEntry)> f) {
    apply = std::move(f);
  }

  void operator()(std::vector<LogEntry> entries) {
    for (auto &&entry : entries) {
      BasicLogEntry basicEntry(entry.get_opName(), entry.get_args(),
                               entry.get_prmIdx());
      applyQueue.addEvent([this, basicEntry = std::move(basicEntry)]() mutable {
        apply(std::move(basicEntry));
      });
    }
  }

  void wait() { applyQueue.wait(); }

private:
  EventQueue applyQueue;
  std::function<void(quintet::BasicLogEntry)> apply = nullptr;
};

} // namespace raft
} // namespace quintet
