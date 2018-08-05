#include <server/timer.h>

#include <functional>
#include <mutex>
#include <boost/thread/thread_only.hpp>

namespace quintet {

struct Timer::Impl {
  std::mutex m;
  std::function<void()> func = nullptr;
  boost::thread th;
  std::uint64_t period = 0;

  bool immediateCache = false;
  bool repeatCache = false;

  void bind(std::uint64_t p, std::function<void()> f);

  void start(bool immediate, bool repeat);

  void stop();

  void restart();

}; // struct Timer::Impl

void Timer::Impl::stop() {
  th.interrupt();
  th.join();
}

void Timer::Impl::restart() {
  stop();
  start(immediateCache, repeatCache);
}

void Timer::Impl::start(bool immediate, bool repeat) {
  assert(period);

  std::unique_lock<std::mutex> lk(m, std::defer_lock);
  if (!lk.try_lock())
    return;

  immediateCache = immediate;
  repeatCache = repeat;

  assert(!th.joinable());
  th = boost::thread([lk = std::move(lk), period = period, f = func, immediate = immediate, repeat = repeat] {
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
}

void Timer::Impl::bind(std::uint64_t p, std::function<void()> f) {
  period = p;
  func = std::move(f);
}

} /* namespace quintet */

namespace quintet {

Timer::Timer() : pImpl(std::make_unique<Impl>()) {
  namespace ph = std::placeholders;
  bind<tag::TimerBind>(std::bind(&Impl::bind, &*pImpl, ph::_1, ph::_2));
  bind<tag::TimerStart>(std::bind(&Impl::start, &*pImpl, ph::_1, ph::_2));
  bind<tag::TimerStop>(std::bind(&Impl::stop, &*pImpl));
  bind<tag::TimerRestart>(std::bind(&Impl::restart, &*pImpl));
}
GEN_PIMPL_DTOR(Timer);

} /* namespace quintet */