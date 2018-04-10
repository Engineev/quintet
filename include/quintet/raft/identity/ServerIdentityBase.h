#ifndef QUINTET_SERVERIDENTITYBASE_H
#define QUINTET_SERVERIDENTITYBASE_H

#include "ServerInfo.h"
#include "ServerState.h"
#include "ServerService.h"
#include "RaftDefs.h"

namespace quintet {

class ServerIdentityBase {
public:
    ServerIdentityBase(ServerState & state_,
                       ServerInfo & info_,
                       ServerService & service_);

    virtual ~ServerIdentityBase() = default;

    /// RPCs

    // Why pure virtual function and default__ are used ?
    // reference: Effective C++, 3rd edition, Term 34
    // Caution: the default version is non-synchronized

    virtual std::pair<Term /*current term*/, bool /*success*/>
    RPCAppendEntries(Term term, ServerId leaderId,
                     std::size_t prevLogIdx, Term prevLogTerm,
                     std::vector<LogEntry> logEntries, std::size_t commitIdx) = 0;

    virtual std::pair<Term /*current term*/, bool /*vote granted*/>
    RPCRequestVote(Term term, ServerId candidateId,
                   std::size_t lastLogIdx, Term lastLogTerm) = 0;

    virtual void leave() {throw; }

    virtual void init() {throw; }

protected:
    ServerState      & state;
    ServerService    & service;
    const ServerInfo & info;

protected:
    // TODO: upToDate: check whether the given RPC info is up to date.
    // This is implemented in ServerState.h
    // bool upToDate(std::size_t lastLogIdx, Term lastLogTerm) const {
    //     return true;
    // }

    std::pair<Term /*current term*/, bool /*success*/>
    defaultRPCAppendEntries(Term term, ServerId leaderId, // TODO
                     std::size_t prevLogIdx, Term prevLogTerm,
                     std::vector<LogEntry> logEntries, std::size_t commitIdx) {throw ;}

    // This is not thread safe
    std::pair<Term /*current term*/, bool /*vote granted*/>
    defaultRPCRequestVote(Term term, ServerId candidateId,
                   std::size_t lastLogIdx, Term lastLogTerm) {
                       throw;
        // if (term < state.currentTerm)
        //      return {state.currentTerm, false};

        // if ((state.votedFor == NullServerId || state.votedFor == candidateId)
        //         && upToDate(lastLogIdx, lastLogTerm)) {
        //     return {state.currentTerm, true};
        // }

        // return {state.currentTerm, false};
    }

}; // class ServerIdentityBase

} // namespace quintet


#endif //QUINTET_SERVERIDENTITYBASE_H
