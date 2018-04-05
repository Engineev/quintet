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
    void init(const std::string & configDir) {
        info.load(configDir);
        initService();
        identities[(std::size_t)ServerIdentityNo::Follower]
                = std::make_unique<ServerIdentityFollower>(state, info, service);
        identities[(std::size_t)ServerIdentityNo::Candidate]
                = std::make_unique<ServerIdentityCandidate>(state, info, service);
        identities[(std::size_t)ServerIdentityNo::Leader]
                = std::make_unique<ServerIdentityLeader>(state, info, service);
        currentIdentity = ServerIdentityNo::Down;

    }

    void run() {
        service.identityTransformer.transform(ServerIdentityNo::Follower);
    }

    void stop() {
        service.identityTransformer.transform(ServerIdentityNo::Down);
    }

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

private:
    std::array<std::unique_ptr<ServerIdentityBase>, 3> identities;
    ServerIdentityNo currentIdentity = ServerIdentityNo::Down;

    ServerState   state;
    ServerInfo    info;
    ServerService service;

private:
    void initService() {
        service.identityTransformer.bind([&](ServerIdentityNo to) {transform(to);});

        service.rpcService.bind("AppendEntries",
                                [&](Term term, ServerId leaderId,
                                    std::size_t prevLogIdx, Term prevLogTerm,
                                    std::vector<LogEntry> logEntries, std::size_t commitIdx) {
                                    return RPCAppendEntries(term, leaderId,  prevLogIdx, prevLogTerm,
                                           std::move(logEntries), commitIdx);
                                });
        service.rpcService.bind("RequestVote",
                                 [&](Term term, ServerId candidateId,
                                     std::size_t lastLogIdx, Term lastLogTerm) {
                                     return RPCRequestVote(term, candidateId, lastLogIdx, lastLogTerm);
                                 });

        service.logger.set("./", info.local.addr + "_" + std::to_string(info.local.port));
    }

    void refreshState() {
        state.votedFor = NullServerId;
    }

    // the following functions should never be invoked directly !!!

    void transform(ServerIdentityNo to) {
        auto from = currentIdentity;
#ifdef IDENTITY_TEST
        currentIdentity = from;
#else
        currentIdentity = to;
#endif

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
};

} // namespace quintet


#endif //QUINTET_SERVER_H
