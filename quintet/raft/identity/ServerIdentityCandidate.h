#ifndef QUINTET_SERVERIDENTITYCANDIDATE_H
#define QUINTET_SERVERIDENTITYCANDIDATE_H

#include <vector>

#include "raft/identity/ServerIdentityBase.h"

namespace quintet {

class ServerIdentityCandidate
        : public ServerIdentityBase {
public:
    ServerIdentityCandidate(ServerState & state_,
                            ServerInfo & info_,
                            ServerService & service_);;

    ~ServerIdentityCandidate() override = default;

    std::pair<Term /*current term*/, bool /*success*/>
    RPCAppendEntries(Term term, ServerId leaderId,
                     std::size_t prevLogIdx, Term prevLogTerm,
                     std::vector<LogEntry> logEntries, std::size_t commitIdx) override {throw ;};

    std::pair<Term /*current term*/, bool /*vote granted*/>
    RPCRequestVote(Term term, ServerId candidateId,
                   std::size_t lastLogIdx, Term lastLogTerm) override {throw ;};

    void leave() override {throw; }

    /// \breif See figure 2 of the paper
    ///
    /// 1. Increase the current term.
    /// 2. Vote for self
    /// 3. Reset election timer
    /// 4. Send RequestVote RPCs to the other servers.
    ///    This procedure will not wait for the other servers to reply.
    void init() override {
        ++state.currentTerm;
        state.votedFor = info.local;
        // TODO: reset election timer
        voteRequested = 1;
        launchVotesChecker(sendRequests());
    }

private:
    std::atomic<std::size_t> voteRequested{0};

    std::vector<FutureWrapper<RPCLIB_MSGPACK::object_handle>> sendRequests();

    void launchVotesChecker(std::vector<FutureWrapper<RPCLIB_MSGPACK::object_handle>> && votes) {

    }

}; // class ServerIdentityCandidate

} // namespace quintet


#endif //QUINTET_SERVERIDENTITYCANDIDATE_H
