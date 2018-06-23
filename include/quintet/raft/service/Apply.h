#pragma once

#include <functional>
#include <vector>

#include "RaftDefs.h"
#include "misc/EventQueue.h"

namespace quintet {

class Apply {
public:
  void bind(std::function<void(quintet::LogEntry)> f) {
    apply = std::move(f);
  }

  void operator()(std::vector<LogEntry> entries) {
    for (auto && entry : entries)
      applyQueue.addEvent([this, entry = std::move(entry)] () mutable {
        apply(std::move(entry));
      });
  }

  void wait() {
    applyQueue.wait();
  }

private:
  EventQueue applyQueue;
  std::function<void(quintet::LogEntry)> apply = nullptr;
};

} /* namespace quintet */
