#ifndef QUINTET_TIMER_H
#define QUINTET_TIMER_H

#include "actor_types.h"
#include "common/macro.h"

namespace quintet {

class Timer : public TimerActor {
public:
  Timer();
  ~Timer();

private:
  GEN_PIMPL_DEF()

}; // class Timer

} // namespace quintet

#endif //QUINTET_TIMER_H
