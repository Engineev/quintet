#ifndef QUINTET_SERVERSERVICE_H
#define QUINTET_SERVERSERVICE_H

/**
 *  The services required to apply the Raft algorithm
 *  such as RPC and identity transformation.
 *  Services:
 *    - IdentityTransformer
 *    - RpcService
 *    - Logger
 *    - HeartBeatController
 */

#include <functional>
#include <mutex>
#include <thread>
#include <vector>
#include <functional>
#include <memory>
#include <condition_variable>
#include <fstream>

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/thread.hpp>

#include <rpc/server.h>
#include <rpc/client.h>

#include "raft/RaftDefs.h"
#include "Utility.h"
#include "Future.h"


// forward declaration
namespace quintet { struct ServerService;  }


// IdentityTransformer
namespace quintet {

class IdentityTransformer {
public:
    void bind(std::function<void(ServerIdentityNo)> transform);

    // Implementation detail:
    // There is no possibility that two identity transformations
    // will be executed during the same identity period since
    // before the transformation being carried out, the RPC
    // service will be stop first. And the RPC service will wait
    // until all the RPCs are completed. As the transformation
    // is asynchronous, the PRC which triggered the transformation
    // will exit almost immediately. And the other PRCs who
    // trying to trigger another transformation will fail since
    // they can not lock the mutex. Since try_lock() is adopted,
    // they will also exit immediately.
    void transform(ServerIdentityNo target);

private:
    std::function<void(ServerIdentityNo /*target*/,
                       std::unique_lock<std::mutex> &&)> transform_;
    std::mutex transforming;
};

} /* namespace quintet */

// RpcService
namespace quintet {

class RpcService {
public:
    /// \brief Change the port to listen on to the given one.
    /// The original server will be stopped first and the
    /// functors bound previously will become invalid.
    ///
    /// \param port the port to listen on.
    void listen(Port port);


    void async_run(std::size_t worker = 1);

    template <class Func>
    RpcService& bind(const std::string & name, Func f) {
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

    /// \brief Create a rpc::client and invoke the corresponding async_call.
    /// The client created will be packed with the boost::future returned so
    /// that it will not be destroyed prematurely.
    template <typename... Args>
    FutureWrapper<RPCLIB_MSGPACK::object_handle>
        async_call(const std::string & addr, Port port, std::string const &func_name, Args... args) {
        auto c = std::make_unique<rpc::client>(addr, port);
        auto fut = c->async_call(func_name, std::move(args)...);
        return {toBoostFuture(std::move(fut)), std::move(c)};
    }

    template <typename... Args>
    RPCLIB_MSGPACK::object_handle call(
            const std::string & addr, Port port,
            std::string const &func_name, Args... args) {
        return async_call(addr, port, func_name, std::move(args)...).get();
    }

private:
    std::unique_ptr<rpc::server> srv;

    boost::shared_mutex rpcing;
    bool paused = false;
    std::mutex pausing;
    std::condition_variable cv;

private:

    template <class Func, class Closure, class Ret, class... Args>
    void bind_impl(const std::string & name, Func rawF, Ret(Closure::*)(Args...) const) {
        srv->bind(name, [rawF, this](Args... args) -> Ret {
            std::unique_lock<std::mutex> pauseLk(pausing);
            cv.wait(pauseLk, [&] { return !paused;});
            boost::shared_lock<boost::shared_mutex> rpcLk(rpcing); // acquire this lock before releasing pauseLk !!
            pauseLk.unlock();
            return rawF(std::move(args)...);
        });
    }

}; // class RpcService

} /* namespace quintet */

// Logger
#define LOGGING
namespace quintet {

class Logger {
public:
    Logger() = default;

    Logger(std::string dir, std::string id);

    ~Logger();

    void set(std::string dir_, std::string id_);

    template <class... Args>
    void log(const Args&... args) {
#ifdef LOGGING
        std::lock_guard<std::mutex> lk(logging);
        fout << id << ": ";
        log_impl(args...);
#endif
    }

    template <class... Args>
    void operator()(const Args&... args) {
        log(args...);
    }

private:
    std::mutex    logging;
    std::string   dir;
    std::string   id;
    std::ofstream fout;

    void log_impl();

    template <class T, class... Args>
    void log_impl(const T & x, const Args&... args) {
        fout << x;
        log_impl(args...);
    };
};

} /* namespace quintet */

// HeartBeatController
namespace quintet {

class HeartBeatController {
public:
    HeartBeatController() = default;

    HeartBeatController(std::function<void()> f, std::uint32_t periodMs);

    ~HeartBeatController();

    void bind(std::function<void()> f, std::uint32_t periodMs_);

    void start();

    /// \brief Sleep for periodMs ms and then invoke f.
    /// Calling stop() will disable all the waiting oneShots.
    ///
    /// TODO: test: oneShot
    void oneShot(std::function<void()> f, std::uint32_t periodMs);

    void stop();

private:
    std::function<void()> heartBeat;
    std::uint32_t         periodMs = 0;
    std::atomic<bool>     running{false};
    boost::thread         beat;

    std::mutex                 launching;
    std::vector<boost::thread> oneShots;

    void run();

}; // class HeartBeatController

} /* namespace quintet */

// Committer
namespace quintet {

class Committer {

};

} /* namespace quintet */

// ServerService
namespace quintet {

struct ServerService {
    IdentityTransformer identityTransformer;
    RpcService          rpcService;
    Logger              logger;
    HeartBeatController heartBeatController;
}; // class ServerService

} // namespace quintet

#endif //QUINTET_SERVERSERVICE_H
