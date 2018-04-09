#ifndef QUINTET_SERVERIDENTITYFOLLOWER_H
#define QUINTET_SERVERIDENTITYFOLLOWER_H

#include <memory>
#include <random>

#include <boost/chrono.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#include "ServerIdentityBase.h"

namespace quintet {

class ServerIdentityFollower : public ServerIdentityBase {
  public:
    ServerIdentityFollower(ServerState &state_, ServerInfo &info_,
                           ServerService &service_);

    ~ServerIdentityFollower() override = default;

    /// 1. Respond to the heart beat from the leader
    /// 2. If logEntries is not empty, append the entries to the log
    std::pair<Term /*current term*/, bool /*success*/>
    RPCAppendEntries(Term term, ServerId leaderId, std::size_t prevLogIdx,
                     Term prevLogTerm, std::vector<LogEntry> logEntries,
                     std::size_t commitIdx) override;

    std::pair<Term /*current term*/, bool /*vote granted*/>
    RPCRequestVote(Term term, ServerId candidateId, std::size_t lastLogIdx,
                   Term lastLogTerm) override;

    void leave() override;

    void init() override;

  private:
    boost::shared_mutex currentTermM;

    std::uint64_t electionTimeout;

    void resetHeartBeat();

}; // class ServerIdentityFollower

} // namespace quintet

#endif // QUINTET_SERVERIDENTITYFOLLOWER_H
