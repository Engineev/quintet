#ifndef QUINTET_SERVERIDENTITYBASE_H
#define QUINTET_SERVERIDENTITYBASE_H

#include "quintet/raft/ServerInfo.h"
#include "quintet/raft/ServerState.h"
#include "quintet/raft/ServerService.h"
#include "quintet/raft/RaftDefs.h"

namespace quintet {

class ServerIdentityBase {
public:
    ServerIdentityBase(ServerState & state_,
                       ServerInfo & info_,
                       ServerService & service_);

    virtual ~ServerIdentityBase() = default;

    /// RPCs

    virtual std::pair<Term /*current term*/, bool /*success*/>
    RPCAppendEntries(Term term, ServerId leaderId,
                     std::size_t prevLogIdx, Term prevLogTerm,
                     std::vector<LogEntry> logEntries, std::size_t commitIdx) {throw; }

    virtual std::pair<Term /*current term*/, bool /*vote granted*/>
    RPCRequestVote(Term term, ServerId candidateId,
                   std::size_t lastLogIdx, Term lastLogTerm) {throw; }

    virtual void leave() {throw; }

    virtual void init() {throw; }

protected:
    ServerState      & state;
    ServerService    & service;
    const ServerInfo & info;

protected:
    // TODO: upToDate
    bool upToDate(std::size_t lastLogIdx, Term lastLogTerm) const {
        throw ;
    }

    std::pair<Term /*current term*/, bool /*vote granted*/>
    defaultRPCRequestVote(Term term, ServerId candidateId,
                   std::size_t lastLogIdx, Term lastLogTerm) {
        if (term < state.currentTerm)
             return {state.currentTerm, false};

        if ((state.votedFor == NullServerId || state.votedFor == candidateId)
                && upToDate(lastLogIdx, lastLogTerm)) {
            return {state.currentTerm, true};
        }

        return {state.currentTerm, false};
    }

}; // class ServerIdentityBase

} // namespace quintet


#endif //QUINTET_SERVERIDENTITYBASE_H
