#include "RpcClients.h"

void quintet::RpcClients::createClients(const std::vector<quintet::ServerId> &srvs) {
    for (auto & c : clients)
        c.second->wait_all_responses();
    clients.clear();
    for (auto & srv : srvs) {
        auto c = std::make_unique<rpc::client>(srv.addr, srv.port);
        clients[srv.toString()] = std::move(c);
    }
}

template<typename... Args>
boost::future<RPCLIB_MSGPACK::object_handle>
quintet::RpcClients::async_call(quintet::ServerId srv, const std::string & name, Args... args) {
    auto & c = *clients.at(srv.toString());
    auto fut = c.async_call(name, std::move(args)...);
    return toBoostFuture(std::move(fut));
}

template<typename... Args>
RPCLIB_MSGPACK::object_handle
quintet::RpcClients::call(quintet::ServerId srv, const std::string & name, Args... args) {
    return async_call(srv, name, std::move(args)...).get();
}

void quintet::RpcClients::stop() {
    createClients({});
}
