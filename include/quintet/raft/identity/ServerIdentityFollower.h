#ifndef QUINTET_SERVERIDENTITYFOLLOWER_H
#define QUINTET_SERVERIDENTITYFOLLOWER_H

#include "raft/identity/ServerIdentityBase.h"

namespace quintet {

class ServerIdentityFollower
        : public ServerIdentityBase {
public:
    ServerIdentityFollower(ServerState & state_,
                           ServerInfo & info_,
                           ServerService & service_);;

    ~ServerIdentityFollower() override = default;

    std::pair<Term /*current term*/, bool /*success*/>
    RPCAppendEntries(Term term, ServerId leaderId,
                     std::size_t prevLogIdx, Term prevLogTerm,
                     std::vector<LogEntry> logEntries, std::size_t commitIdx) override {throw; }

    std::pair<Term /*current term*/, bool /*vote granted*/>
    RPCRequestVote(Term term, ServerId candidateId,
                   std::size_t lastLogIdx, Term lastLogTerm) override {throw; }

    void leave() override {throw; }

    void init() override {throw; }

private:


}; // class ServerIdentityFollower

} // namespace quintet


#endif //QUINTET_SERVERIDENTITYFOLLOWER_H