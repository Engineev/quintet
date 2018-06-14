#include "identity/IdentityBase.h"

#include <boost/thread/lock_guard.hpp>

namespace quintet {

IdentityBase::IdentityBaseImpl::IdentityBaseImpl(ServerState &state,
                                                 ServerInfo &info,
                                                 ServerService &service)
    : state(state), info(info), service(service) {}

AddLogReply IdentityBase::IdentityBaseImpl::defaultAddLog(AddLogMessage) {
  boost::lock_guard<ServerState> lk(state);
  return {false, state.currentLeader};
}

} // namespace quintet