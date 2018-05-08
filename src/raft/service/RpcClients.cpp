#include "RpcClients.h"

void quintet::RpcClients::createClients(const std::vector<quintet::ServerId> &srvs,  std::uint64_t timeOut) {
    for (auto & c : clients)
        c.second->wait_all_responses();
    clients.clear();
    for (auto & srv : srvs) {
        auto c = std::make_unique<rpc::client>(srv.addr, srv.port);
        c->set_timeout(timeOut);
        clients[srv.toString()] = std::move(c);
    }
}

void quintet::RpcClients::stop() {
    createClients({});
}
