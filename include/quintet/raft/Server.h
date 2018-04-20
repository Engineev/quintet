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

class Server {
public:
    void init(const std::string & configDir);

    void bindCommit(std::function<void(LogEntry)> commit);

    void run();

    /// \breif stop the server and exit all running threads
    void stop();

private: /// RPCs
    std::pair<Term /*current term*/, bool /*success*/>
    RPCAppendEntries(Term term, ServerId leaderId,
                     std::size_t prevLogIdx, Term prevLogTerm,
                     std::vector<LogEntry> logEntries, std::size_t commitIdx);

    std::pair<Term /*current term*/, bool /*vote granted*/>
    RPCRequestVote(Term term, ServerId candidateId,
                   std::size_t lastLogIdx, Term lastLogTerm);;


private:
    std::array<std::unique_ptr<ServerIdentityBase>, 3> identities;
    ServerIdentityNo currentIdentity = ServerIdentityNo::Down;

    ServerState   state;
    ServerInfo    info;
    ServerService service;
    RpcService    rpc;

private:
    void initService();

    void refreshState();

    // the following functions should never be invoked directly !!!

    void transform(ServerIdentityNo to);

#ifdef IDENTITY_TEST
public:
    void setTestedIdentity(ServerIdentityNo no) {
        testedIdentity = no;
    };

    // return the identity the tester wants to transform to
    void setOnTransform(std::function<ServerIdentityNo(ServerIdentityNo from, ServerIdentityNo to)> f) {
        onTransform = std::move(f);
    }

    std::uint64_t getElectionTimeout() const {
        return info.electionTimeout;
    }

    ServerIdentityNo /*from*/ setIdentity_test(ServerIdentityNo to) {
        auto from = currentIdentity;
        currentIdentity = to;

        rpc.pause();
        service.heartBeatController.stop();

        if (from != ServerIdentityNo::Down)
            identities[(std::size_t)from]->leave();

        refreshState();

        if (to != ServerIdentityNo::Down) {
            identities[(std::size_t) to]->init();
            service.heartBeatController.start();
            rpc.resume();
        }
        return from;
    }

    ServerIdentityNo currentIdentity_test() const {
        return currentIdentity;
    }

    void transform_test(ServerIdentityNo to) {
        auto actual = onTransform(currentIdentity, to);

        service.logger("transform from ", IdentityNames[(int)currentIdentity],
            " to ", IdentityNames[(int)to], ". Actually to ", IdentityNames[(int)actual]);

        setIdentity_test(actual);
    }

private:
    ServerIdentityNo testedIdentity = ServerIdentityNo::Down;
    std::function<ServerIdentityNo(ServerIdentityNo from, ServerIdentityNo to)> onTransform;

#endif
};

} // namespace quintet


#endif //QUINTET_SERVER_H
