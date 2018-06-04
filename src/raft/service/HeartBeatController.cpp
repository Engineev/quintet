#include "service/HeartBeatController.h"

#include <boost/chrono.hpp>
#include <boost/thread/locks.hpp>

namespace quintet {

void HeartBeatController::stop() {
  th.interrupt();
  th.join();
}

void HeartBeatController::restart() {
  stop();
  start(immediateCache, repeatCache);
}

bool HeartBeatController::start(bool immediate, bool repeat) {
  assert(period);

  boost::unique_lock<boost::mutex> lk(m, boost::defer_lock);
  if (!lk.try_lock()) {
    return false;
  }

  immediateCache = immediate;
  repeatCache = repeat;

  assert(!th.joinable());
  th = boost::thread([lk = std::move(lk), period = period, f = func,
                         immediate = immediate, repeat = repeat] {
    boost::this_thread::disable_interruption di;
    if (immediate) {
      f();
      if (!repeat)
        return;
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

void HeartBeatController::bind(
    std::uint64_t periodMs, std::function<void()> f) {
  func = std::move(f);
  period = periodMs;
}

HeartBeatController::~HeartBeatController() { stop(); }

} // namespace quintet