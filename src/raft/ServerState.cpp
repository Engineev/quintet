#include "ServerState.h"

#include <boost/thread/locks.hpp>

namespace quintet {

Term ServerState::get_currentTerm() const {
  boost::shared_lock<boost::shared_mutex> lk(currentTermM);
  return currentTerm;
}

Index ServerState::get_commitIdx() const {
  boost::shared_lock<boost::shared_mutex> lk(commitIdxM);
  return commitIdx;
}
} // namespace quintet