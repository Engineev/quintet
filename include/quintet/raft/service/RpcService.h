#ifndef QUINTET_RPCSERVICE_H
#define QUINTET_RPCSERVICE_H

#include <functional>
#include <memory>
#include <condition_variable>
#include <mutex>
#include <atomic>

#include <boost/thread/shared_mutex.hpp>
#include <boost/log/sources/logger.hpp>

#include "Future.h"
#include "Utility.h"
#include "RaftDefs.h"
#include "log/Common.h"

#include <rpc/server.h>
#include <rpc/client.h>

// RpcService
namespace quintet {


class RpcService {
public:
    void configLogger(const std::string &id);

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

    void pause();

    /// \brief Resume the paused RPC service and notify all the RPCs waiting.
    void resume();

private:
    logging::src::logger_mt lg;
    std::unique_ptr<rpc::server> srv;

    // sync stop() and rpc
    // pause() will not influence the increasing of numRpcRemaining
    std::mutex stopping;
    std::atomic<std::size_t> numRpcRemaining{0};
    std::condition_variable modifiedNumRpcRemaining;

    // sync pause()/resume() and rpc
    boost::shared_mutex rpcing;
    std::mutex paused;

private:
    template<class Func, class Closure, class Ret, class... Args>
    void bind_impl(const std::string &name, Func rawF,
                   Ret (Closure::*)(Args...) const) {
        srv->bind(name, [rawF, this](Args... args) -> Ret {
            ++numRpcRemaining;
            std::unique_lock<std::mutex> plk(paused);
            boost::shared_lock<boost::shared_mutex> lk(rpcing);
            plk.unlock();

            std::shared_ptr<void> defer(nullptr, [this](void *) {
                --numRpcRemaining;
                modifiedNumRpcRemaining.notify_all();
            });
            return rawF(std::move(args)...);
        });
    }

}; // class RpcService

} /* namespace quintet */

#endif //QUINTET_RPCSERVICE_H
