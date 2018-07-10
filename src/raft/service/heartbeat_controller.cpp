#include "raft/service/heartbeat_controller.h"

#include <boost/thread/mutex.hpp>
#include <boost/thread/thread_only.hpp>

#include "misc/macro.h"

namespace quintet {
namespace raft {

struct HeartbeatController::Impl {
  boost::mutex m;
  std::function<void()> func = nullptr;
  boost::thread th;
  std::uint64_t period = 0;

  bool immediateCache = false;
  bool repeatCache = false;
}; // struct HeartbeatController::Impl

} // namespace raft
} // namespace quintet

namespace quintet {
namespace raft {

GEN_PIMPL_CTOR(HeartbeatController)
HeartbeatController::~HeartbeatController() {
  stop();
}

void HeartbeatController::stop() {
  pImpl->th.interrupt();
  pImpl->th.join();
}

void HeartbeatController::restart() {
  stop();
  start(pImpl->immediateCache, pImpl->repeatCache);
}

bool HeartbeatController::start(bool immediate, bool repeat) {
  assert(pImpl->period);

  boost::unique_lock<boost::mutex> lk(pImpl->m, boost::defer_lock);
  if (!lk.try_lock()) {
    return false;
  }

  pImpl->immediateCache = immediate;
  pImpl->repeatCache = repeat;

  assert(!pImpl->th.joinable());
  pImpl->th =
      boost::thread([lk = std::move(lk), period = pImpl->period,
                     f = pImpl->func, immediate = immediate, repeat = repeat] {
        boost::this_thread::disable_interruption di;
        if (immediate) {
          f();
          if (!repeat) return;
        }
        do {
          try {
            boost::this_thread::restore_interruption ri(di);
            boost::this_thread::sleep_for(boost::chrono::milliseconds(period));
          } catch (boost::thread_interrupted) {
            return;
          }
          f();
        } while (repeat);
      });
  return true;
}

void HeartbeatController::bind(std::uint64_t periodMs,
                               std::function<void()> f) {
  pImpl->func = std::move(f);
  pImpl->period = periodMs;
}

} // namespace raft
} // namespace quintet