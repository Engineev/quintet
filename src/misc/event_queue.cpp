#include "misc/event_queue.h"

#include <queue>

#include <boost/thread/mutex.hpp>
#include <boost/thread/thread_only.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/atomic.hpp>

namespace quintet {

struct EventQueue::Impl {
  struct Node {
    std::size_t idx;
    std::function<void()> event;
  };

  std::queue<Node> q;
  boost::thread runningThread;
  boost::atomic_bool paused{false};
  boost::mutex m;
  boost::condition_variable cv;
  std::size_t curIdx = 0;

  void addEvent(std::function<void()> event) {
    boost::lock_guard<boost::mutex> lk(m);
    q.push({++curIdx, std::move(event)});
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
      auto node = q.front();
      q.pop();
      lk.unlock();

      {
        boost::this_thread::disable_interruption d;
        node.event();
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
  }

  void stop() {
    resume();
    wait();
    runningThread.interrupt();
    runningThread.join();
  }

  void start() {
    if (runningThread.joinable())
      return;
    runningThread = boost::thread(std::bind(&EventQueue::Impl::execute, this));
  }
};

EventQueue::EventQueue() : pImpl(std::make_unique<EventQueue::Impl>()) {
  pImpl->start();
}

EventQueue::~EventQueue() { stop(); }

void EventQueue::addEvent(std::function<void()> event) {
  pImpl->addEvent(std::move(event));
}

void EventQueue::wait() { pImpl->wait(); }

void EventQueue::stop() { pImpl->stop(); }

void EventQueue::start() { pImpl->start(); }


} // namespace quintet