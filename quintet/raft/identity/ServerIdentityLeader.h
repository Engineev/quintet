#ifndef QUINTET_SERVERIDENTITYLEADER_H
#define QUINTET_SERVERIDENTITYLEADER_H

#include "raft/identity/ServerIdentityBase.h"

namespace quintet {

class ServerIdentityLeader
        : public ServerIdentityBase {
public:
    ~ServerIdentityLeader() override ;

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


}; // class ServerIdentityLeader

} // namespace quintet


#endif //QUINTET_SERVERIDENTITYLEADER_H
