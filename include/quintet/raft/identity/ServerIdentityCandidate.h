#ifndef QUINTET_SERVERIDENTITYCANDIDATE_H
#define QUINTET_SERVERIDENTITYCANDIDATE_H

#include <vector>
#include <memory>

#include "ServerIdentityBase.h"


namespace quintet {

class ServerIdentityCandidate
        : public ServerIdentityBase {
public:
    ServerIdentityCandidate(ServerState & state_,
                            ServerInfo & info_,
                            ServerService & service_);

    ~ServerIdentityCandidate() override;

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
    struct Impl;
    std::unique_ptr<Impl> pImpl;

#ifdef UNIT_TEST
private:
    struct TestImpl;
    std::unique_ptr<TestImpl> tpImpl;
#endif

}; // class ServerIdentityCandidate

} // namespace quintet


#endif //QUINTET_SERVERIDENTITYCANDIDATE_H
