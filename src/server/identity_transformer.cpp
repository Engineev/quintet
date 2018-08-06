#include <server/identity_transformer.h>

#include <atomic>
#include <future>

namespace quintet {

struct IdentityTransformer::Impl {
  RaftActor::Mailbox toRaft;
  std::atomic<bool> triggered{false};
  std::thread runningThread;

  void trigger(IdentityNo target);

  void reset();

}; // struct IdentityTransformer::Impl

void IdentityTransformer::Impl::trigger(IdentityNo target) {
  if (triggered.exchange(true))
    return;
  if (runningThread.joinable())
    runningThread.join();
  runningThread = std::thread([this, target] {
    toRaft.send<tag::TransformIdentity>(target).get();
  });
}

void IdentityTransformer::Impl::reset() { triggered = false; }

} /* namespace quintet */

namespace quintet {

IdentityTransformer::IdentityTransformer() : pImpl(std::make_unique<Impl>()) {
  namespace ph = std::placeholders;
  bind<tag::TriggerTransform>(std::bind(&Impl::trigger, &*pImpl, ph::_1));
  bind<tag::ResetTransformer>(std::bind(&Impl::reset, &*pImpl));
}
IdentityTransformer::~IdentityTransformer() {
  if (pImpl->runningThread.joinable())
    pImpl->runningThread.join();
}

void IdentityTransformer::bindMailboxes(RaftActor::Mailbox toRaft) {
  pImpl->toRaft = std::move(toRaft);
}

} /* namespace quintet */