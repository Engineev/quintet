#ifndef QUINTET_SERVERIDENTITYFOLLOWER_H
#define QUINTET_SERVERIDENTITYFOLLOWER_H

#include <random>
#include <memory>

#include <boost/chrono.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

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
                   std::size_t lastLogIdx, Term lastLogTerm) override {
        boost::upgrade_lock<boost::upgrade_mutex> lk(currentTermM);
        if (term < state.currentTerm)
            return {state.currentTerm, false};
        if (term > state.currentTerm) {
            auto ulk = boost::upgrade_to_unique_lock<boost::upgrade_mutex>(lk);
            state.currentTerm = term;
        }

        if ((state.votedFor == NullServerId || state.votedFor == candidateId)
            && upToDate(lastLogIdx, lastLogTerm)) {
            state.votedFor = candidateId;
            return {state.currentTerm, true};
        }
        return {state.currentTerm, false};
    }

    void leave() override;

    void init() override;

private:
    boost::upgrade_mutex currentTermM;


}; // class ServerIdentityFollower

} // namespace quintet


#endif //QUINTET_SERVERIDENTITYFOLLOWER_H
