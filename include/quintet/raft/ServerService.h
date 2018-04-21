#ifndef QUINTET_SERVERSERVICE_H
#define QUINTET_SERVERSERVICE_H

#include <functional>
#include <condition_variable>
#include <mutex>

#include <boost/thread/shared_mutex.hpp>

#include <rpc/client.h>
#include <rpc/server.h>

#include "Future.h"
#include "Utility.h"
#include "RaftDefs.h"

#include "HeartBeatController.h"
#include "IdentityTransformer.h"
#include "log/Common.h"

// RpcService
namespace quintet {

/* The states of RpcService
 * 1. Down
 * 2. Running
 * 3. Paused
 *
 *
 */
class RpcService {
public:
    /// \brief Change the port to listen on to the given one.
    /// The original server will be stopped first and the
    /// functors bound previously will become invalid.
    ///
    /// \param port the port to listen on.
    void listen(Port port);

    void async_run(std::size_t worker = 1);

    /// \breif stop the RPC service. All the ongoing RPCs will
    ///        be finished first.
    ///
    /// Currently, please make sure that the RpcService is running
    /// when stop() is invoked.
    void stop();

    template<class Func>
    RpcService &bind(const std::string &name, Func f) {
        bind_impl(name, f, &Func::operator());
        return *this;
    }

    /* The detail of the implementation of pause() and resume()
     * There are two things we are required to guarantee:
     * 1. pause() will block until all the RPC running is completed.
     * 2. After being paused, all the RPCs came will block until
     *    resume() has been invoked.
     *
     * *. (async_)call is still available after being paused.
     *
     * We can use read-write locks and boost::shared_mutex to handle
     * the first requirement. And a flag "paused" is used to handle
     * the second one. Meanwhile, we use a std::mutex to synchronize
     * the read and modification of the flag and a
     * std::conditional_variable to wake the waiting RPCs.
     */

    /// \brief Pause the RPC service until resume() is invoked.
    /// The original connections will not be invalid. After being
    /// paused, the RPCs will wait.
    /// Block until the current RPCs are completed.
    ///
    /// The functionality of pause is actually implemented in bind()
    /// TODO: "After being paused..." -> "Right after pause() being invoked..."
    void pause();

    /// \brief Resume the paused RPC service and notify all the RPCs waiting.
    void resume();

private:
    std::unique_ptr<rpc::server> srv;

    boost::shared_mutex rpcing;
    bool paused = false;
    std::mutex pausing;
    std::condition_variable cv;

private:
    template<class Func, class Closure, class Ret, class... Args>
    void bind_impl(const std::string &name, Func rawF,
                   Ret (Closure::*)(Args...) const) {
        srv->bind(name, [rawF, this](Args... args) -> Ret {
            std::unique_lock<std::mutex> pauseLk(pausing);
            cv.wait(pauseLk, [&] { return !paused; });
            boost::shared_lock<boost::shared_mutex> rpcLk(
                    rpcing); // acquire this lock before releasing pauseLk !!
            pauseLk.unlock();
            return rawF(std::move(args)...);
        });
    }

}; // class RpcService

} /* namespace quintet */

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
    logging::src::logger logger;
    Committer committer;
}; // class ServerService

} // namespace quintet

#endif // QUINTET_SERVERSERVICE_H
