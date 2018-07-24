#ifndef CONCERTO_STATE_MACHINE_H
#define CONCERTO_STATE_MACHINE_H

#include <functional>
#include <memory>


namespace quintet {
namespace server {

class StateMachine {
public:
  StateMachine();
  ~StateMachine();

  StateMachine& bindSave(std::function<void()> save_);
  StateMachine& bindLoad(std::function<void()> load_);

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;

}; // class StateMachine

} // namespace server
} // namespace quintet

#endif //CONCERTO_STATE_MACHINE_H
