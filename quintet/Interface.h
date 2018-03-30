#ifndef QUINTET_INTERFACE_H
#define QUINTET_INTERFACE_H

/**
 *  Interface:
 *    construct --> configure & bind --> run
 *              --> call & asyncCall --> stop --> destroy
 *    Only run(), call(), asynCall() and stop() are thread-safe.
 *    All other things should be done before starting to run or after being stopped
 *    Construction, including move construction and assignment are not thread-safe
 *
 *  The APIs of Consensus
 *    1. void AddLog(std::string opName, std::string args, quintet::PrmIdx idx);
 *    2. void BindCommitter(
 *               std::function<void(
 *                       std::string,     // opName
 *                       std::string,     // args
 *                       ServerId,        // the id of the server which added the log
 *                       quintet::PrmIdx) // the promise to be set
 *                     > committer)
 *    4. ServerId Local() const; // a trait should be provided
 *    5. void Configure(const std::string & filename);
 *    6. void run();
 *    7. void stop();
 *
 */

#include <string>
#include <unordered_map>
#include <functional>
#include <utility>
#include <future>
#include <type_traits>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
//#include <iostream> // debug

#include <boost/serialization/utility.hpp>
#include <boost/hana/if.hpp>
#include <boost/any.hpp>

#include "Serialization.h"
#include "Defs.h"
#include "Utility.h"
#include "Future.h"
#include "SharedInterface.h"

namespace quintet {

// TODO: mimic the smart pointers

template <class Consensus>
class Interface {
public:
    using SrvId = typename Consensus::ServerId;

public:
    Interface() = default;

    GEN_DELETE_COPY(Interface)

    Interface(Interface && o) {
        move_impl(std::move(o));
    }

    Interface& operator=(Interface && o) {
        if (this == &o)
            return *this;
        move_impl(std::move(o));
        return *this;
    }

    ~Interface() {
        if (consensus.get() == nullptr)
            return;
        stop(); 
    }

    void configure(const std::string & filename) {
        consensus->Configure(filename);
        consensus->BindCommitter(
                [&](std::string opName, std::string args, SrvId srvAdd, PrmIdx idx) {
                    return commit(std::move(opName), std::move(args), srvAdd, idx);
                });
        localId = consensus->Local();
    }

    void run() {
        std::lock_guard<std::mutex> lk(turning);
        if (running)
            return;
        running = true;
        consensus->run();
    }

    void stop() {
        std::lock_guard<std::mutex> lk(turning);
        consensus->stop();
        running = false;
    }

    // User should guarantee that the types of arguments here exactly match
    // the ones previously bounded. Otherwise the serialization/deserialization
    // may fail.
    // If you are considering replacing boost::any with the actual type,
    // please figure out the way to dispatch these heterogeneous functions first.
    template <class... Args>
    boost::future<boost::any> asyncCall(const std::string & opName, Args... rawArgs) {
        std::lock_guard<std::mutex> lk(turning);
        if (!running)
            throw ; // TODO: throw something meaningful

        std::string args = serialize(rawArgs...);
        PrmIdx idx = prmIdx++;
        boost::promise<boost::any> prm;
        auto fut = prm.get_future();
        prms.emplace(idx, std::move(prm));

        std::shared_ptr<void> addLog(nullptr, [&] (void*) {
            consensus->AddLog(opName, std::move(args), idx);
        });

        return fut;
    }

    template <class... Args>
    boost::any call(const std::string & opName, Args... args) {
        return asyncCall(opName, std::move(args)...).get();
    }

    template <class Func>
    Interface& bind(const std::string & name, Func f) {
        bind_impl(name, f, &Func::operator());
        return *this; // return *this to enable chained binding.
    }

private:
    // There is no need to save the data the Interface maintained,
    // because if the Interface is down, all the consumers are all down.

    // the mapping from the names of the operation to the corresponding function
    std::unordered_map<std::string, std::function<boost::any(std::string)>> fs;
    // the promises to be set
    std::unordered_map<PrmIdx, boost::promise<boost::any>> prms;
    std::atomic<PrmIdx> prmIdx{0};

    std::unique_ptr<Consensus>  consensus;
    SrvId      localId;
    std::mutex committing;

    std::mutex turning;
    bool       running;

    void move_impl(Interface && o) {
        fs        = std::move(o.fs);
        prms      = std::move(o.prms);
        prmIdx    = o.prmIdx;
        consensus = std::move(o.consensus);
        running   = false;
    }

private:
    // The only use of the third parameter is to get the types of the arguments explicitly.
    template <class Func, class Closure, class Ret, class... Args>
    void bind_impl(const std::string & name, Func rawF, Ret(Closure::*)(Args...) const) {
        fs.insert(std::make_pair(name, [rawF = rawF](std::string rawArgs)->boost::any {
            auto tup = deserialize<Args...>(rawArgs);
            auto res = boost::hana::unpack(tup, rawF);
            return res;
        }));
    }

    template <class Func, class Closure, class... Args>
    void bind_impl(const std::string & name, Func rawF, void(Closure::*)(Args...) const) {
        fs.insert(std::make_pair(name, [rawF = rawF](std::string rawArgs)->boost::any {
            auto tup = deserialize<Args...>(rawArgs);
            boost::hana::unpack(tup, rawF);
            return boost::any();
        }));
    }

    // TODO: sync ??
    void commit(std::string opName, std::string args, SrvId srvAdd, PrmIdx idx) {
        std::lock_guard<std::mutex> lk(committing);
        auto res = fs.at(opName)(std::move(args));
        if (srvAdd == localId) {
            auto prm = prms.find(idx);
            assert(prm != prms.end());
            prm->second.set_value(res);
            prms.erase(prm);
        }
    }


}; // class Interface

} // namespace quintet

#endif //QUINTET_INTERFACE_H
