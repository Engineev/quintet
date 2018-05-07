#ifndef QUINTET_RPCCLIENTS_H
#define QUINTET_RPCCLIENTS_H

#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <unordered_map>

#include <rpc/client.h>

#include "ServerInfo.h"
#include "Future.h"

namespace quintet {

class RpcClients {
public:
    /// \brief Clear the current clients and create new ones
    ///
    /// The ongoing calls will be finished first.
    ///
    /// \param srvs The servers which the clients created
    ///             connected to.
    void createClients(const std::vector<ServerId> & srvs);

    template <typename... Args>
    boost::future<RPCLIB_MSGPACK::object_handle> async_call(
        ServerId srv, const std::string & name, Args... args);

    template <typename... Args>
    RPCLIB_MSGPACK::object_handle call(
        ServerId srv, const std::string & name, Args... args);

    void stop();

private:
    std::unordered_map<std::string, std::unique_ptr<rpc::client>> clients;

}; // class RpcClients

} // namespace quintet


#endif //QUINTET_RPCCLIENTS_H
