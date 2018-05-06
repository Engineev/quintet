// Maybe this file can be merged into Raft.h

#ifndef QUINTET_SERVER_H
#define QUINTET_SERVER_H

#include <array>
#include <memory>
#include <mutex>
#include <utility>
#include <atomic>
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

public:
    bool localAppendEntries(std::vector<LogEntry> logEntries);

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
    logging::src::logger_mt rpcLg;


    boost::mutex  transforming;
    Term          termTransformed = InvalidTerm;
    boost::thread transformThread;

private:
    bool triggerTransformation(ServerIdentityNo target, Term term);

    void transform(ServerIdentityNo target);

private:
    void initService();

    void refreshState();


// IDENTITY_TEST BEGIN
public:
    // return the identity the tester wants to transform to
    void setBeforeTransform(std::function<ServerIdentityNo(ServerIdentityNo from, ServerIdentityNo to)> f);

    void setAfterTransform(std::function<void(ServerIdentityNo from, ServerIdentityNo to)> f);

    std::uint64_t getElectionTimeout() const;

    ServerIdentityNo getCurrentIdentity() const;

    /// \brief send empty RPCAppendEntries to other servers. (sync)
    void sendHeartBeat();

    void setRpcLatency(std::uint64_t lb, std::uint64_t ub) {
        rpcLatencyLb = lb;
        rpcLatencyUb = ub;
    }

    ServerInfo getInfo() const {
        return info;
    }

private:
    std::function<ServerIdentityNo(ServerIdentityNo from, ServerIdentityNo to)> beforeTransform;
    std::function<void(ServerIdentityNo from, ServerIdentityNo to)> afterTransform;
    std::atomic<uint64_t> rpcLatencyLb{0}, rpcLatencyUb{0};
// IDENTITY_TEST END
};

} // namespace quintet


#endif //QUINTET_SERVER_H
