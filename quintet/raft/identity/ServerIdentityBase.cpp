#include "ServerIdentityBase.h"

quintet::ServerIdentityBase::ServerIdentityBase(quintet::ServerState &state_, quintet::ServerInfo &info_,
                                                quintet::ServerService &service_)
        : state(state_), service(service_), info(info_) {}
