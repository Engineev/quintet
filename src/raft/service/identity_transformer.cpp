#include "raft/service/identity_transformer.h"

#include <boost/atomic/atomic.hpp>

#include "misc/macro.h"

namespace quintet {
namespace raft {

GEN_PIMPL_CTOR(IdentityTransformer);
IdentityTransformer::~IdentityTransformer() = default;

struct IdentityTransformer::Impl {
  std::function<void(IdentityNo)> triggerTransform = nullptr;
  boost::atomic_bool triggered{false};
};

bool IdentityTransformer::notify(IdentityNo target) {
  if (pImpl->triggered.exchange(true))
    return false;
  pImpl->triggerTransform(target);
  return false;
}

void IdentityTransformer::bind(std::function<void(IdentityNo)> slot) {
  pImpl->triggerTransform = std::move(slot);
}

void IdentityTransformer::reset() {
  pImpl->triggered = false;
}

} // namespace raft
} // namespace quintet