#ifndef QUINTET_SERVERIDENTITYBASE_H
#define QUINTET_SERVERIDENTITYBASE_H

#include <memory>

#include "RaftDefs.h"
#include "ServerInfo.h"
#include "ServerService.h"
#include "ServerState.h"

namespace quintet {

struct IdentityBaseImpl {
    IdentityBaseImpl(ServerState & state,
                     ServerInfo & info,
                     ServerService &service);

    ServerState & state;
    ServerService &service;
    const ServerInfo &info;
};

class ServerIdentityBase {
public:
    virtual ~ServerIdentityBase() = default;

    // client's requests
    virtual bool localAppendEntries(std::vector<LogEntry> logEntries) {
        throw ;
    }

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
}; // class ServerIdentityBase

} // namespace quintet

#endif // QUINTET_SERVERIDENTITYBASE_H
