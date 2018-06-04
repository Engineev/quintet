#include "service/IdentityTransformer.h"

#include <functional>

#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>

namespace quintet {

IdentityTransformer::IdentityTransformer() : pImpl(std::make_unique<Impl>()) {}

IdentityTransformer::~IdentityTransformer() = default;

struct IdentityTransformer::Impl {
  std::function<void(ServerIdentityNo target)> triggerTransform = nullptr;
  boost::mutex m;
  Term termTriggered = InvalidTerm;
};

bool IdentityTransformer::notify(ServerIdentityNo target, Term currentTerm) {
  boost::lock_guard<boost::mutex> lk(pImpl->m);
  if (pImpl->termTriggered == InvalidTerm ||
      pImpl->termTriggered < currentTerm) {
    pImpl->termTriggered = currentTerm;
    pImpl->triggerTransform(target);
    return true;
  }
  return false;
}

} // namespace quintet