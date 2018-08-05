#include <server/identity_transformer.h>

#include <atomic>
#include <future>

namespace quintet {

struct IdentityTransformer::Impl {
  RaftActor::Mailbox toRaft;
  std::atomic<bool> triggered{false};
  std::future<void> last;

  void trigger(IdentityNo target);

  void reset();

}; // struct IdentityTransformer::Impl

void IdentityTransformer::Impl::trigger(IdentityNo target) {
  if (triggered.exchange(true))
    return;
  if (last.valid())
    last.get();
  last = toRaft.send<tag::TransformIdentity>(target);
}

void IdentityTransformer::Impl::reset() { triggered = false; }

} /* namespace quintet */

namespace quintet {

IdentityTransformer::IdentityTransformer() : pImpl(std::make_unique<Impl>()) {
  namespace ph = std::placeholders;
  bind<tag::TriggerTransform>(std::bind(&Impl::trigger, &*pImpl, ph::_1));
  bind<tag::ResetTransformer>(std::bind(&Impl::reset, &*pImpl));
}
GEN_PIMPL_DTOR(IdentityTransformer)

void IdentityTransformer::bindMailboxes(RaftActor::Mailbox toRaft) {
  pImpl->toRaft = std::move(toRaft);
}

} /* namespace quintet */