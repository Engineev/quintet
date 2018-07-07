#ifndef QUINTET_SERVER_H
#define QUINTET_SERVER_H

#include <memory>
#include <functional>
#include <string>

#include <boost/any.hpp>

#include "misc/serialization.h"

namespace quintet {

class Server {
public:
  Server();
  ~Server();

  void Configure();

  void Start();

  void Wait();

  void Shutdown();

  template <class Func> Server & Bind(const std::string &name, Func f) {
    return bindImpl(name, std::move(makeFunc(f, &Func::operator())));
  }

private:
  template <class Func, class Closure, class Ret, class... Args>
  std::function<boost::any(std::string)>
  makeFunc(Func f, Ret(Closure::*)(Args...) const) {
    return [f](std::string rawArgs) -> boost::any {
      auto tup = deserialize<Args...>(rawArgs);
      auto res = boost::hana::unpack(tup, f);
      return res;
    };
  }

  template <class Func, class Closure, class... Args>
  std::function<boost::any(std::string)>
  makeFunc(Func f, void(Closure::*)(Args...) const) {
    return [f](std::string rawArgs) -> boost::any {
      auto tup = deserialize<Args...>(rawArgs);
      boost::hana::unpack(tup, f);
      return boost::any();
    };
  };

  Server & bindImpl(const std::string & name,
                    std::function<boost::any(std::string)> f);

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
}; // class Server

} // namespace quintet

#endif // QUINTET_SERVER_H
