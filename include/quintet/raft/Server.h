#ifndef QUINTET_SERVER_H
#define QUINTET_SERVER_H

#include <memory>
#include <string>
#include <vector>
#include <utility>

#include "RaftDefs.h"
#include "ServerInfo.h"

namespace quintet {

class Server {
public:
    Server();

    ~Server();

    void init(const std::string & configDir);

    void bindCommit(std::function<void(LogEntry)> commit);

    void run();

    /// \breif stop the server and exit all running threads
    void stop();

    bool localAppendEntries(std::vector<LogEntry> logEntries) {}

private: /// RPCs
    std::pair<Term /*current term*/, bool /*success*/>
    RPCAppendEntries(Term term, ServerId leaderId,
                     std::size_t prevLogIdx, Term prevLogTerm,
                     std::vector<LogEntry> logEntries, std::size_t commitIdx);

    std::pair<Term /*current term*/, bool /*vote granted*/>
    RPCRequestVote(Term term, ServerId candidateId,
                   std::size_t lastLogIdx, Term lastLogTerm);;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;

private:
    void initRpcServer();

    void initServerService();

    void refreshState();

    bool triggerTransformation(ServerIdentityNo target, Term term);

    void transform(quintet::ServerIdentityNo target);

    /// Wait until the current transformation (if exists) finishes
    void waitTransformation();

#ifdef UNIT_TEST
public:
    void setBeforeTransform(std::function<
        ServerIdentityNo(ServerIdentityNo from, ServerIdentityNo to)> f);

    void setAfterTransform(std::function<
        void(ServerIdentityNo from, ServerIdentityNo to)> f);

    void setRpcLatency(std::uint64_t lb, std::uint64_t ub);

    void rpcSleep();

    ServerInfo getInfo() const;

    Term getCurrentTerm() const;

private:
    struct TestImpl;
    std::unique_ptr<TestImpl> tpImpl;

#endif
}; // class Server

} // namespace quintet

#endif //QUINTET_SERVER_H
