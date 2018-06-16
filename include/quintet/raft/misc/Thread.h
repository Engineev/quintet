#ifndef QUINTET_THREAD_H
#define QUINTET_THREAD_H

#include <memory>
#include <boost/thread/thread_only.hpp>

namespace quintet {

class ThreadPool {
public:
  ThreadPool();

  ~ThreadPool();

  void add(boost::thread && t);

  void clearWithInterruption();

  std::size_t size() const;

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
};

} // namespace quintet

#endif //QUINTET_THREAD_H
