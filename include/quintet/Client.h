#ifndef QUINTET_CLIENT_H
#define QUINTET_CLIENT_H

#include <memory>

#include <boost/any.hpp>
#include <boost/thread/future.hpp>

#include "common.h"
#include "client_context.h"
#include "misc/serialization.h"
#include "misc/object.h"

namespace quintet {

class Client {
public:
  explicit Client(ServerId target);

  template <class... Args>
  boost::future<Object> asyncCall(ClientContext ctx, std::string opName,
                                  Args... args) {
    return asyncCallImpl(ctx, std::move(opName), serialize(args...));
  }

  template <class... Args>
  boost::future<Object> asyncCall(std::string opName, Args... args) {
    return asyncCall(ClientContext(), std::move(opName), std::move(args)...);
  }

  template <class... Args>
  Object call(ClientContext ctx, std::string opName, Args... args) {
    return asyncCall(ctx, std::move(opName), std::move(args)...).get();
  }

  template <class... Args>
  Object call(std::string opName, Args... args) {
    return call(ClientContext(), std::move(opName), std::move(args)...);
  }

private:
  boost::future<Object> asyncCallImpl(ClientContext ctx, std::string opName,
                                      std::string args);

  class Impl;
  std::unique_ptr<Impl> pImpl;

}; // class Client

} // namespace quintet

#endif //QUINTET_CLIENT_H
