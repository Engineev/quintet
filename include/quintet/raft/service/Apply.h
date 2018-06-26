#pragma once

#include <functional>
#include <vector>

#include "RaftDefs.h"
#include "misc/EventQueue.h"

namespace quintet {

class Apply {
public:
  void bind(std::function<void(quintet::BasicLogEntry)> f) {
    apply = std::move(f);
  }

  void operator()(std::vector<LogEntry> entries) {
    for (auto && entry : entries)
      applyQueue.addEvent([this, entry = std::move(entry)] () mutable {
        BasicLogEntry basicEntry(entry.opName, entry.args,
                                 entry.prmIdx, entry.srvId);
        apply(std::move(basicEntry));
      });
  }

  void wait() {
    applyQueue.wait();
  }

private:
  EventQueue applyQueue;
  std::function<void(quintet::BasicLogEntry)> apply = nullptr;
};

} /* namespace quintet */
