#pragma once

#include <functional>

#include "RaftDefs.h"

namespace quintet {

class Apply {
public:
  void bind(std::function<void(quintet::LogEntry)> f) {
    apply = std::move(f);
  }

  void operator()(LogEntry log) {
    apply(std::move(log));
  }

private:
  std::function<void(quintet::LogEntry)> apply = nullptr;
};

} /* namespace quintet */
