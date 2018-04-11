#ifndef QUINTET_SERVERIDENTITYBASE_H
#define QUINTET_SERVERIDENTITYBASE_H

#include "RaftDefs.h"
#include "ServerInfo.h"
#include "ServerService.h"
#include "ServerState.h"

namespace quintet {

class ServerIdentityBase {
public:
    ServerIdentityBase(ServerState &state_, ServerInfo &info_,
                       ServerService &service_);

    virtual ~ServerIdentityBase() = default;

    /// RPCs

    virtual std::pair<Term /*current term*/, bool /*success*/>
    RPCAppendEntries(Term term, ServerId leaderId, std::size_t prevLogIdx,
                     Term prevLogTerm, std::vector<LogEntry> logEntries,
                     std::size_t commitIdx) = 0;

    virtual std::pair<Term /*current term*/, bool /*vote granted*/>
    RPCRequestVote(Term term, ServerId candidateId, std::size_t lastLogIdx,
                   Term lastLogTerm) = 0;

    virtual void leave() { throw; }

    virtual void init() { throw; }

protected:
    ServerState &state;
    ServerService &service;
    const ServerInfo &info;
    boost::upgrade_mutex entriesM;
}; // class ServerIdentityBase

} // namespace quintet

#endif // QUINTET_SERVERIDENTITYBASE_H
