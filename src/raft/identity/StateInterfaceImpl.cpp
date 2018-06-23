#include "identity/StateInterfaceImpl.h"

#include <boost/thread/shared_lock_guard.hpp>
#include <boost/thread/locks.hpp>

namespace quintet {

const Term StateInterfaceImpl::get_currentTerm() const {
  boost::shared_lock_guard<ServerState> LK(state);
  boost::shared_lock_guard<boost::shared_mutex> lk(currentTermM);
  return state.get_currentTerm();
}

bool StateInterfaceImpl::set_currentTerm(std::function<bool(Term)> condition,
                                         Term term) {
  boost::shared_lock_guard<ServerState> LK(state);
  boost::lock_guard<boost::shared_mutex> lk(currentTermM);
  if (condition(state.get_currentTerm())) {
    state.getMutable_currentTerm() = term;
    return true;
  }
  return false;
}

} // namespace quintet