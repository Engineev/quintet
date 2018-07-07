#ifndef QUINTET_EVENT_QUEUE_H
#define QUINTET_EVENT_QUEUE_H

#include <memory>
#include <functional>
#include <string>

namespace quintet {

class EventQueue {
public:
  EventQueue();

  ~EventQueue();

  void addEvent(std::function<void()> event);

  void wait();

  /// \brief Graceful shutdown
  void stop();

  void start();

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
}; // class EventQueue

} // namespace quintet

#endif //QUINTET_EVENT_QUEUE_H
