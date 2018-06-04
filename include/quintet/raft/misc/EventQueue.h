#ifndef QUINTET_EVENTQUEUE_H
#define QUINTET_EVENTQUEUE_H

#include <memory>
#include <functional>

namespace quintet {

class EventQueue {
public:
  EventQueue();

  ~EventQueue();

  // pseudo non-blocking
  void addEvent(std::function<void()> event);

  void wait();

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
};

} // namespace quintet

#endif //QUINTET_EVENTQUEUE_H
