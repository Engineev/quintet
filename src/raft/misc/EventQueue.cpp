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
  boost::mutex produceM, consumeM;
  boost::condition_variable produceCv, consumeCv;

  std::queue<std::function<void()>> q;
  boost::thread runningThread;
  boost::atomic_bool paused{false};

  void addEvent(std::function<void()> event) {
    boost::lock_guard<boost::mutex> lk(produceM);
    q.push(std::move(event));
    produceCv.notify_all();
  }

  void execute() {
    while (true) {
      boost::unique_lock<boost::mutex> plk(produceM, boost::defer_lock);
      boost::unique_lock<boost::mutex> clk(consumeM, boost::defer_lock);
      try {
        boost::lock(plk, clk);
        produceCv.wait(plk, [this] { return !paused && !q.empty(); });
      } catch (boost::thread_interrupted & e) {
        return;
      }
      auto exe = q.front();
      q.pop();
      plk.unlock();

      {
        boost::this_thread::disable_interruption d;
        exe();
      }
      consumeCv.notify_all();
    }
  }

  void pause() { paused = true; }

  void resume() {
    paused = false;
    consumeCv.notify_all();
  }

  void wait() {
    boost::unique_lock<boost::mutex> lk(consumeM);
    consumeCv.wait(lk, [this] { return q.empty(); });
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

void EventQueue::pause() { pImpl->pause(); }

void EventQueue::resume() { pImpl->resume(); }

void EventQueue::stop() { pImpl->stop(); }

void EventQueue::start() { pImpl->start(); }

} // namespace quintet