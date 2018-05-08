#ifndef QUINTET_RPCCLIENTS_H
#define QUINTET_RPCCLIENTS_H

#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <unordered_map>
#include <stdexcept>

#include <rpc/client.h>
#include <rpc/server.h>
#include <rpc/rpc_error.h>

#include "ServerInfo.h"
#include "Future.h"

namespace quintet {

class RpcDisconnected : public std::exception {};

class RpcClients {
public:
    /// \brief Clear the current clients and create new ones
    ///
    /// The ongoing calls will be finished first.
    ///
    /// \param srvs The servers which the clients created
    ///             connected to.
    void createClients(const std::vector<ServerId> & srvs, std::uint64_t timeOut = 1000);

    template <typename... Args>
    boost::future<RPCLIB_MSGPACK::object_handle> async_call(
        ServerId srv, const std::string & name, Args... args) {
        auto & c = *clients.at(srv.toString());
//        if (c.get_connection_state() != rpc::client::connection_state::connected)
//            throw RpcDisconnected();
        auto fut = c.async_call(name, std::move(args)...);
        return toBoostFuture(std::move(fut));
    }

    template <typename... Args>
    RPCLIB_MSGPACK::object_handle call(
        ServerId srv, const std::string & name, Args... args) {
        return async_call(srv, name, std::move(args)...).get();
    }

    void stop();

private:
    std::unordered_map<std::string, std::unique_ptr<rpc::client>> clients;

}; // class RpcClients

} // namespace quintet


#endif //QUINTET_RPCCLIENTS_H
