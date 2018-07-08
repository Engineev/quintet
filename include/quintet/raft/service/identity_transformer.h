#ifndef QUINTET_IDENTITY_TRANSFORMER_H
#define QUINTET_IDENTITY_TRANSFORMER_H

#include <functional>
#include <memory>

#include "raft/raft_common.h"

namespace quintet {
namespace raft {

// lock-free
class IdentityTransformer {
public:
  IdentityTransformer();

  ~IdentityTransformer();

  void bind(std::function<void(IdentityNo)> slot);

  bool notify(IdentityNo target);

  void reset();

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
}; // class IdentityTransformer

} // namespace raft
} // namespace quintet

#endif //QUINTET_IDENTITY_TRANSFORMER_H
