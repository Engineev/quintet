#ifndef QUINTET_SERVERIDENTITYCANDIDATE_H
#define QUINTET_SERVERIDENTITYCANDIDATE_H

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

    void init() override {throw; }

private:


}; // class ServerIdentityCandidate

} // namespace quintet


#endif //QUINTET_SERVERIDENTITYCANDIDATE_H
