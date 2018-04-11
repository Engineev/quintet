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
                     std::vector<LogEntry> logEntries, std::size_t commitIdx) override;;

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
    /// 1. Discard all the remaining RequestVote RPCs
    /// 2. Reset the shared pointers
    void leave() override;


private:
    /* Why shared pointer are used here ?
     * I think that two different term should be completely independent,
     * which means that the operations in different term will never affect
     * the same object. Therefore, I use this std::shared_ptr trick.
     * At init(), new shared pointers are create and be acquired by the
     * threads launched during this term. And at leave(), the shared pointers
     * which stored in the object will be cleaned so that the resource will
     * be released after all the other threads launched during this term
     * exist.
     */

    struct ElectionData {
        boost::mutex m;
        bool         discarded = false;
        std::size_t  votesReceived = 1;
    };

    std::shared_ptr<ElectionData> data;

private:
    /// \brief Send RPCRequestVotes to other servers and
    ///        count the votes
    ///
    /// \param electionTerm The term at which this election started
    void requestVotes(Term electionTerm);

#ifdef IDENTITY_TEST
    /// \brief Notify the other servers the end of the election
    void notifyReign() {
        for (auto & srv : info.srvList) {
            if (srv == info.local)
                continue;
            boost::thread([srv, this] {
                rpc::client c(srv.addr, srv.port);
                // TODO: call -> send ??
                c.call("AppendEntries", 0, ServerId(), 0, 0, std::vector<LogEntry>(), 0);
                service.logger("Shutdown ", srv);
            }).detach();
        }
    }
#endif


}; // class ServerIdentityCandidate

} // namespace quintet


#endif //QUINTET_SERVERIDENTITYCANDIDATE_H
