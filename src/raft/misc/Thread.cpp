#include "misc/Thread.h"

#include <list>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

namespace quintet {

struct ThreadPool::Impl {
  std::list<boost::thread> ts;
  mutable boost::mutex m;

  void add(boost::thread && t) {
    boost::lock_guard<boost::mutex> lk(m);
    ts.emplace_back(std::move(t));
  }

  void clearWithInterruption() {
    boost::lock_guard<boost::mutex> lk(m);
    for (auto & t : ts)
      t.interrupt();
    for (auto & t : ts)
      t.join();
    ts.clear();
  }

  std::size_t size() const {
    boost::lock_guard<boost::mutex> lk(m);
    return ts.size();
  }

}; // struct ThreadPool::Impl

ThreadPool::ThreadPool() : pImpl(std::make_unique<Impl>()) {}

ThreadPool::~ThreadPool() = default;

void ThreadPool::add(boost::thread &&t) { pImpl->add(std::move(t)); }

void ThreadPool::clearWithInterruption() { pImpl->clearWithInterruption(); }

std::size_t ThreadPool::size() const { return pImpl->size(); }

} // namespace quintet
