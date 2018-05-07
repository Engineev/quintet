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

void quintet::RpcClients::stop() {
    createClients({});
}
