#include "ServerIdentityBase.h"

quintet::ServerIdentityBase::ServerIdentityBase(quintet::ServerState &state_, quintet::ServerInfo &info_,
                                                quintet::ServerService &service_)
        : state(state_), info(info_), service(service_) {}
