#ifndef QUINTET_STATE_MACHINE_H
#define QUINTET_STATE_MACHINE_H

#include <functional>
#include <memory>

#include <theatre/actor.h>
#include <theatre/string.h>

namespace quintet {
namespace {
  using SaveT = THEATRE_TAG("save");
  using LoadT = THEATRE_TAG("load");
}

class StateMachine  {
public:
  StateMachine();
  ~StateMachine();

private:
  void save();
  void load();

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;

}; // class StateMachine

} // namespace quintet

#endif //QUINTET_STATE_MACHINE_H
