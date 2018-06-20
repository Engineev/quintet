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

  void pause();

  void resume();

  /// \brief Graceful shutdown
  void stop();

  void start();

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
};

} // namespace quintet

#endif //QUINTET_EVENTQUEUE_H
