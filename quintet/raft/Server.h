// Maybe this file can be merged into Raft.h

#ifndef QUINTET_SERVER_H
#define QUINTET_SERVER_H

#include <array>
#include <memory>
#include <mutex>
#include <thread>

#include "quintet/raft/identity/ServerIdentity.h"
#include "quintet/raft/ServerInfo.h"
#include "quintet/raft/ServerState.h"
#include "quintet/raft/ServerService.h"


namespace quintet {

// TODO: thread-safe: event-driven, all the sync operations should be done at the service level ?
// TODO: bind
class Server {
public:
    void configure();

    /// \brief
    void run() {
        service.identityTransformer.transform(ServerIdentityNo::Follower);
    }

    void stop() {
        service.identityTransformer.transform(ServerIdentityNo::Down);
    }

private:
    std::array<std::unique_ptr<ServerIdentityBase>, 3> identities;
    ServerIdentityNo currentIdentity = ServerIdentityNo::Down;

    ServerState   state;
    ServerInfo    info;
    ServerService service;

private:
    void initBind() {
        service.identityTransformer.bind([&](ServerIdentityNo to) {transform(to);});
    }

    // the following should never be invoked directly !!!

    void transform(ServerIdentityNo to) {
        auto from = currentIdentity;
        currentIdentity = to;

        if (from != ServerIdentityNo::Down)
            identities[(std::size_t)from]->leave();
        if (to != ServerIdentityNo::Down)
            identities[(std::size_t)to]->init();
    }
};

} // namespace quintet


#endif //QUINTET_SERVER_H
