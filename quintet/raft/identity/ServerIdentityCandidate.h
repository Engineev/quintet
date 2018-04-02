#ifndef QUINTET_SERVERIDENTITYCANDIDATE_H
#define QUINTET_SERVERIDENTITYCANDIDATE_H

#include <vector>
#include <memory>
#include <cassert>

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

    /// \breif See figure 2 of the paper
    ///
    /// 1. Increase the current term.
    /// 2. Vote for self
    /// 3. Reset election timer (TODO:)
    /// 4. Send RequestVote RPCs to the other servers.
    ///    This procedure will not wait for the other servers to reply.
    void init() override {
        ++state.currentTerm;
        state.votedFor = info.local;

        // TODO: reset election timer

        data = std::make_shared<ElectionData>();
        launchVotesChecker(sendRequests());
    }

    /// \brief clean up
    ///
    /// 1. Discard all the remaining RequestVote RPCs
    /// 2. Reset the shared pointers
    void leave() override {
        boost::unique_lock<boost::mutex> lk(data->m);

        data->discarded = true;
        // Unlock before setting m to nullptr, otherwise it
        // may happen that this thread is the last thread
        // which hold m and set m to nullptr will destroy
        // the mutex before unlocking it.
        lk.unlock();
        data = nullptr;
    }

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
    std::vector<FutureWrapper<RPCLIB_MSGPACK::object_handle>> sendRequests();

    void launchVotesChecker(std::vector<FutureWrapper<RPCLIB_MSGPACK::object_handle>> && votes);

}; // class ServerIdentityCandidate

} // namespace quintet


#endif //QUINTET_SERVERIDENTITYCANDIDATE_H
