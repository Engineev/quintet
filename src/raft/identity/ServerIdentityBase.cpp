#include "ServerIdentityBase.h"

namespace quintet {

IdentityBaseImpl::IdentityBaseImpl(
    ServerState &state, ServerInfo &info, ServerService &service)
    : state(state), service(service), info(info) {}

} // namespace quintet