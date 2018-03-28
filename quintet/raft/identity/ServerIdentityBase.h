#ifndef QUINTET_SERVERIDENTITYBASE_H
#define QUINTET_SERVERIDENTITYBASE_H

#include "quintet/raft/ServerInfo.h"
#include "quintet/raft/ServerState.h"
#include "quintet/raft/ServerService.h"
#include "quintet/raft/RaftDefs.h"

namespace quintet {

class ServerIdentityBase {
public:
    virtual ~ServerIdentityBase() = default;

    /// RPCs

    virtual std::pair<Term /*current term*/, bool /*success*/>
    RPCAppendEntries(Term term, ServerId leaderId,
                     std::size_t prevLogIdx, Term prevLogTerm,
                     std::vector<LogEntry> logEntries, std::size_t commitIdx);

    virtual std::pair<Term /*current term*/, bool /*vote granted*/>
    RPCRequestVote(Term term, ServerId candidateId,
                   std::size_t lastLogIdx, Term lastLogTerm);

    virtual void leave();

    virtual void init();

private:
    ServerState   & state;
    ServerInfo    & info;
    ServerService & service;

}; // class ServerIdentityBase

} // namespace quintet


#endif //QUINTET_SERVERIDENTITYBASE_H
