#ifndef QUINTET_INTERFACE_H
#define QUINTET_INTERFACE_H

#include <string>
#include <unordered_map>
#include <functional>
#include <utility>
#include <future>
#include <type_traits>
#include <thread>
#include <iostream> // debug

#include <boost/serialization/utility.hpp>
#include <boost/hana/if.hpp>
#include <boost/any.hpp>

#include "quintet/Serialization.h"

namespace quintet {

template <class Consensus>
class Interface {
public:

    // User should guarantee that the types of arguments here exactly match
    // the ones previously bounded. Otherwise the serialization/deserialization
    // may fail.
    // TODO: call: replace boost::any with the actual type & more type-check ?
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
    Consensus consensus;

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


}; // class Interface

} // namespace quintet

#endif //QUINTET_INTERFACE_H
