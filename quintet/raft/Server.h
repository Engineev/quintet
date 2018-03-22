// Maybe this file can be merged into Raft.h

#ifndef QUINTET_SERVER_H
#define QUINTET_SERVER_H

#include <array>
#include <memory>

#include "quintet/raft/identity/ServerIdentity.h"
#include "quintet/raft/ServerInfo.h"
#include "quintet/raft/ServerState.h"
#include "quintet/raft/ServerService.h"


namespace quintet {

// TODO: thread-safe
class Server {
public:
    Server(); // TODO: bind RPCs & transformation, configure

private:
    std::array<std::unique_ptr<ServerIdentityBase>, 3> identities;
    ServerIdentityNo currentIdentity = ServerIdentityNo::Error;

    ServerState   state;
    ServerInfo    info;
    ServerService service;

private:

};

} // namespace quintet


#endif //QUINTET_SERVER_H
