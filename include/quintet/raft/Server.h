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
#include "RaftDefs.h"

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

    boost::mutex  transforming;
    boost::thread transformThread;

private:
    bool triggerTransformation(ServerIdentityNo target);

    void transform(ServerIdentityNo target);

private:
    void initService();

    void refreshState();


// IDENTITY_TEST BEGIN
public:
    // return the identity the tester wants to transform to
    void setOnTransform(std::function<ServerIdentityNo(ServerIdentityNo from, ServerIdentityNo to)> f);

    std::uint64_t getElectionTimeout() const;

    ServerIdentityNo getCurrentIdentity() const;

private:
    std::function<ServerIdentityNo(ServerIdentityNo from, ServerIdentityNo to)> onTransform;
// IDENTITY_TEST END
};

} // namespace quintet


#endif //QUINTET_SERVER_H
