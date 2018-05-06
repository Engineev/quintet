#ifndef QUINTET_RPCSERVICE_H
#define QUINTET_RPCSERVICE_H

#include <functional>
#include <memory>
#include <condition_variable>
#include <mutex>

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

    void pause() {
        BOOST_LOG(lg) << "pause";
        paused.lock();
        boost::unique_lock<boost::shared_mutex> lk(rpcing);
    }

    /// \brief Resume the paused RPC service and notify all the RPCs waiting.
    void resume() {
        BOOST_LOG(lg) << "resume";
        paused.unlock();
    }

private:
    std::unique_ptr<rpc::server> srv;
    std::mutex paused;
    boost::shared_mutex rpcing;

    logging::src::logger_mt lg;

private:
    template<class Func, class Closure, class Ret, class... Args>
    void bind_impl(const std::string &name, Func rawF,
                   Ret (Closure::*)(Args...) const) {
        srv->bind(name, [rawF, this](Args... args) -> Ret {
            boost::unique_lock<std::mutex> lk(paused);
            boost::shared_lock<boost::shared_mutex> rpcLk(
                rpcing); // acquire this lock before releasing pauseLk !!
            lk.unlock();
            return rawF(std::move(args)...);
        });
    }

}; // class RpcService

} /* namespace quintet */

#endif //QUINTET_RPCSERVICE_H
