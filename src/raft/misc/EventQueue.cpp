#include "misc/EventQueue.h"

#include <queue>

#include <boost/thread/mutex.hpp>
#include <boost/thread/thread_only.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/atomic.hpp>

namespace quintet {

struct EventQueue::Impl {
  boost::mutex m;
  boost::condition_variable cv;
  std::queue<std::function<void()>> q;
  boost::thread runningThread;

  void addEvent(std::function<void()> event) {
    boost::lock_guard<boost::mutex> lk(m);
    q.push(std::move(event));
    cv.notify_all();
  }

  void execute() {
    while (true) {
      boost::unique_lock<boost::mutex> lk(m);
      try {
        cv.wait(lk, [this] { return !q.empty(); });
      } catch (boost::thread_interrupted & e) {
        return;
      }
      auto exe = q.front();
      q.pop();
      lk.unlock();

      {
        boost::this_thread::disable_interruption d;
        exe();
      }
      cv.notify_all();
    }
  }

  void wait() {
    boost::unique_lock<boost::mutex> lk(m);
    cv.wait(lk, [this] { return q.empty(); });
    lk.unlock();
  }
};

EventQueue::EventQueue() : pImpl(std::make_unique<EventQueue::Impl>()) {
  pImpl->runningThread = boost::thread([this] {
    pImpl->execute();
  });
}

EventQueue::~EventQueue() {
  wait();
  pImpl->runningThread.interrupt();
  pImpl->runningThread.join();
}

void EventQueue::addEvent(std::function<void()> event) {
  pImpl->addEvent(std::move(event));
}

void EventQueue::wait() { pImpl->wait(); }

} // namespace quintet