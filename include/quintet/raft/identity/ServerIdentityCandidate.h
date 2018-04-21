#ifndef QUINTET_SERVERIDENTITYCANDIDATE_H
#define QUINTET_SERVERIDENTITYCANDIDATE_H

#include <cassert>
#include <vector>
#include <memory>
#include <random>

#include <boost/thread/thread.hpp>

#include "ServerIdentityBase.h"
#include "Future.h"

namespace quintet {

class ServerIdentityCandidate
        : public ServerIdentityBase {
public:
    ServerIdentityCandidate(ServerState & state_,
                            ServerInfo & info_,
                            ServerService & service_);

    ~ServerIdentityCandidate() override = default;

    std::pair<Term /*current term*/, bool /*success*/>
    RPCAppendEntries(Term term, ServerId leaderId,
                     std::size_t prevLogIdx, Term prevLogTerm,
                     std::vector<LogEntry> logEntries, std::size_t commitIdx) override;

    std::pair<Term /*current term*/, bool /*vote granted*/>
    RPCRequestVote(Term term, ServerId candidateId,
                   std::size_t lastLogIdx, Term lastLogTerm) override;

    /// \breif See figure 2 of the paper
    ///
    /// 1. Increase the current term.
    /// 2. Vote for self
    /// 3. Reset election timer
    /// 4. Send RequestVote RPCs to the other servers.
    ///    This procedure will not wait for the other servers to reply.
    void init() override;

    /// \brief clean up
    ///
    /// 1. Interrupt all the remaining RequestVote RPCs
    void leave() override;

private:
    std::atomic<std::size_t>      votesReceived{0};
    std::vector<boost::thread>    requestingThreads;

private:
    /// \brief Send RPCRequestVotes to other servers and count the votes
    void requestVotes(Term currentTerm, ServerId local, Index lastLogIdx, Term lastLogTerm);

    std::pair<Term, bool> sendRequestVote(ServerId target,
                                          Term currentTerm, ServerId local, Index lastLogIdx, Term lastLogTerm);
}; // class ServerIdentityCandidate

} // namespace quintet


#endif //QUINTET_SERVERIDENTITYCANDIDATE_H
