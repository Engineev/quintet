#ifndef QUINTET_STATEINTERFACEIMPL_H
#define QUINTET_STATEINTERFACEIMPL_H

#include "StateInterface.h"
#include "StateInterfaceImpl.h"

#include <boost/thread/shared_mutex.hpp>

namespace quintet {

struct StateInterfaceImpl {
  explicit StateInterfaceImpl(ServerState &state) : state(state) {}

  ServerState &state;
  mutable boost::shared_mutex currentTermM;

  const Term get_currentTerm() const;

  bool set_currentTerm(std::function<bool(Term)> condition, Term term);
};

} // namespace quintet

#endif //QUINTET_STATEINTERFACEIMPL_H
