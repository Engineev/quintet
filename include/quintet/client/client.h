#ifndef QUINTET_CLIENT_H
#define QUINTET_CLIENT_H

#include <string>
#include <future>

#include "serialization.h"
#include "common/macro.h"
#include "common/client_context.h"

namespace quintet {

class Client {
public:
  /// \breif Let \p be the path to the configuration file.
  /// create a Client with the configuration.
  Client(const std::string & filename);
  ~Client();

  template <class... Args>
  Object call(ClientContext ctx, std::string opName, Args... args) {
    return callImpl(ctx, std::move(opName), std::move(args)...);
  }

  template <class... Args>
  Object call(std::string opName, Args... args) {
    return call(ClientContext(), std::move(opName), std::move(args)...);
  }

private:
  Object callImpl(ClientContext ctx,
                  const std::string & opName, const std::string & args);

  GEN_PIMPL_DEF();

}; // class Client

} // namespace quintet

#endif //QUINTET_CLIENT_H
