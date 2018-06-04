#ifndef QUINTET_EVENTQUEUE_H
#define QUINTET_EVENTQUEUE_H

#include <memory>
#include <functional>

namespace quintet {

class EventQueue {
public:
  EventQueue();

  ~EventQueue();

  // pesudo non-blocking
  void addEvent(std::function<void()> event) { throw; }

  void wait() {throw ;}

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
};

} // namespace quintet

#endif //QUINTET_EVENTQUEUE_H
