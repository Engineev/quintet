// Maybe this file can be merged into Raft.h

#ifndef QUINTET_SERVER_H
#define QUINTET_SERVER_H

#include <array>
#include <memory>
#include <mutex>
#include <thread>

#include "ServerIdentity.h"
#include "ServerInfo.h"
#include "ServerState.h"
#include "ServerService.h"

namespace quintet {

// TODO: Server: .h -> .cpp
// TODO: thread-safe: event-driven, all the sync operations should be done at the service level ?
// TODO: bind
class Server {
public:
    void init(const std::string & configDir);

    void run();

    void stop();

private: /// RPCs
    std::pair<Term /*current term*/, bool /*success*/>
    RPCAppendEntries(Term term, ServerId leaderId,
                     std::size_t prevLogIdx, Term prevLogTerm,
                     std::vector<LogEntry> logEntries, std::size_t commitIdx) {
        throw ;
    }

    std::pair<Term /*current term*/, bool /*vote granted*/>
    RPCRequestVote(Term term, ServerId candidateId,
                   std::size_t lastLogIdx, Term lastLogTerm) {
        throw ;
    };

    // TODO: uncomment: I commented the #ifdef to enable the linter
//#ifdef IDENTITY_TEST
    void setIdentity(ServerIdentityNo to) {
        auto from = currentIdentity;
        currentIdentity = to;

        service.rpcService.pause();
        service.heartBeatController.stop();

        if (from != ServerIdentityNo::Down)
            identities[(std::size_t)from]->leave();

        refreshState();

        if (to != ServerIdentityNo::Down)
            identities[(std::size_t)to]->init();

        service.heartBeatController.start();
        service.rpcService.resume();
    }
//#endif


private:
    std::array<std::unique_ptr<ServerIdentityBase>, 3> identities;
    ServerIdentityNo currentIdentity = ServerIdentityNo::Down;

    ServerState   state;
    ServerInfo    info;
    ServerService service;

private:
    void initService();

    void refreshState();

    // the following functions should never be invoked directly !!!

    void transform(ServerIdentityNo to);
};

} // namespace quintet


#endif //QUINTET_SERVER_H
