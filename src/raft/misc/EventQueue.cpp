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
  boost::atomic_bool paused{false};

  void addEvent(std::function<void()> event) {
    boost::lock_guard<boost::mutex> lk(m);
    q.push(std::move(event));
    cv.notify_all();
  }

  void execute() {
    while (true) {
      boost::unique_lock<boost::mutex> lk(m);
      try {
        cv.wait(lk, [this] { return !paused && !q.empty(); });
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

  void pause() { paused = true; }

  void resume() {
    paused = false;
    cv.notify_all();
  }

  void wait() {
    boost::unique_lock<boost::mutex> lk(m);
    cv.wait(lk, [this] { return q.empty(); });
    lk.unlock();
  }

  void stop() {
    resume();
    wait();
    runningThread.interrupt();
    runningThread.join();
  }
};

EventQueue::EventQueue() : pImpl(std::make_unique<EventQueue::Impl>()) {
  pImpl->runningThread = boost::thread([this] {
    pImpl->execute();
  });
}

EventQueue::~EventQueue() { stop(); }

void EventQueue::addEvent(std::function<void()> event) {
  pImpl->addEvent(std::move(event));
}

void EventQueue::wait() { pImpl->wait(); }

void EventQueue::pause() { pImpl->pause(); }

void EventQueue::resume() { pImpl->resume(); }

void EventQueue::stop() { pImpl->stop(); }

} // namespace quintet