#ifndef QUINTET_COMMITTER_H
#define QUINTET_COMMITTER_H

#include <functional>

#include "RaftDefs.h"

namespace quintet {

class Committer {
public:
  void bindCommit(std::function<void(LogEntry)> f) {
    commit_ = std::move(f);
  }

  void commit(LogEntry log) {
    commit_(std::move(log));
  }

private:
  std::function<void(LogEntry)> commit_ = {};
};

} /* namespace quintet */

#endif //QUINTET_COMMITTER_H
