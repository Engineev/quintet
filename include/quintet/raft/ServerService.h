#ifndef QUINTET_SERVERSERVICE_H
#define QUINTET_SERVERSERVICE_H

#include <functional>
#include <condition_variable>
#include <mutex>

#include <boost/thread/shared_mutex.hpp>

#include "Future.h"
#include "Utility.h"
#include "RaftDefs.h"

#include "HeartBeatController.h"
#include "IdentityTransformer.h"
#include "RpcService.h"
#include "RpcClients.h"
#include "log/Common.h"



// Committer
namespace quintet {

class Committer {
public:
    void bindCommit(std::function<void(LogEntry)> f);

    void commit(LogEntry log);

private:
    std::function<void(LogEntry)> commit_;
};

} /* namespace quintet */

// ServerService
namespace quintet {

struct ServerService {
    IdentityTransformer  identityTransformer;
    HeartBeatController  heartBeatController;
    RpcClients           rpcClients;
    logging::src::logger logger;
    Committer committer;
}; // class ServerService

} // namespace quintet

#endif // QUINTET_SERVERSERVICE_H
