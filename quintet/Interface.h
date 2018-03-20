#ifndef QUINTET_INTERFACE_H
#define QUINTET_INTERFACE_H

/**
 *  The APIs of Consensus
 *  1. std::future<boost::any> AddLog(std::string opName, std::string args);
 *  2. void BindCommitter(
 *             std::function<void(
 *                     std::string,        // opName
 *                     std::string,        // args
 *                     quintet::ServerId,  // the id of the server which added the log
 *                     quintet::PrmIdx)>)  // the promise to be set
 *  3. ServerId local() const; // a trait should be provided
 */

#include <string>
#include <unordered_map>
#include <functional>
#include <utility>
#include <future>
#include <type_traits>
#include <thread>
#include <mutex>
//#include <iostream> // debug

#include <boost/serialization/utility.hpp>
#include <boost/hana/if.hpp>
#include <boost/any.hpp>

#include "quintet/Serialization.h"
#include "quintet/Defs.h"
#include "quintet/Utility.h"

namespace quintet {

template <class Consensus>
class Interface {
public:
//    using ServerId = typename Consensus::ServerId;

public:
    Interface() = default;

    GEN_DELETE_COPY_AND_MOVE(Interface)

    void configure(const std::string & filename);

    explicit Interface(ServerId local) : localId(std::move(local)) {
        consensus.BindCommitter(
                [&](std::string opName, std::string args, ServerId srvAdd, PrmIdx idx) {
            commit(std::move(opName), std::move(args), srvAdd, idx);
        });
    }



    // User should guarantee that the types of arguments here exactly match
    // the ones previously bounded. Otherwise the serialization/deserialization
    // may fail.
    template <class... Args>
    std::future<boost::any> asyncCall(const std::string & opName, Args... args) {
        std::string log = serialize(args...);
        return consensus.AddLog(opName, log);
    }

    template <class... Args>
    boost::any call(const std::string & opName, Args... args) {
        auto res = asyncCall(opName, args...);
        return res.get();
    }

    template <class Func>
    void bind(const std::string & name, Func f) {
        bind_impl(name, f, &Func::operator());
    }

private:
    // the mapping from the names of the operation to the corresponding function
    std::unordered_map<std::string, std::function<boost::any(std::string)>> fs;
    // the promises to be set
    std::unordered_map<PrmIdx, std::promise<boost::any>> prms;
    Consensus consensus;

    std::mutex committing;

    using

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

    void commit(std::string opName, std::string args, ServerId srvAdd, PrmIdx idx) {
        std::lock_guard<std::mutex> lk(committing);
        auto res = fs.at(opName)(std::move(args));
        if (srvAdd == localId) {
            auto prm = prms.find(idx);
            prm->second.set_value(res);
            prms.erase(prm);
        }
    }


}; // class Interface

} // namespace quintet

#endif //QUINTET_INTERFACE_H
