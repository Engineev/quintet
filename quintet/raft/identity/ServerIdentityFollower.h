#ifndef QUINTET_SERVERIDENTITYFOLLOWER_H
#define QUINTET_SERVERIDENTITYFOLLOWER_H

#include "raft/identity/ServerIdentityBase.h"

namespace quintet {

class ServerIdentityFollower
        : public ServerIdentityBase {
public:
    ~ServerIdentityFollower() override ;

    std::pair<Term /*current term*/, bool /*success*/>
    RPCAppendEntries(Term term, ServerId leaderId,
                     std::size_t prevLogIdx, Term prevLogTerm,
                     std::vector<LogEntry> logEntries, std::size_t commitIdx) override ;

    std::pair<Term /*current term*/, bool /*vote granted*/>
    RPCRequestVote(Term term, ServerId candidateId,
                   std::size_t lastLogIdx, Term lastLogTerm) override ;

    void leave() override;

    void init() override;

private:


}; // class ServerIdentityFollower

} // namespace quintet


#endif //QUINTET_SERVERIDENTITYFOLLOWER_H
