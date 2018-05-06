#include "RpcService.h"

#include <boost/log/attributes.hpp>
#include <boost/log/attributes/named_scope.hpp>

void quintet::RpcService::listen(quintet::Port port) {
    BOOST_LOG(lg) << "listen on " << port;
    if (srv)
        srv->stop();
    srv = std::make_unique<rpc::server>(port);
}

void quintet::RpcService::async_run(size_t worker) {
    srv->async_run(worker);
}

void quintet::RpcService::stop() {
    BOOST_LOG(lg) << "stop";
    if (srv)
        srv->stop();
}

void quintet::RpcService::configLogger(const std::string &id) {
    using namespace logging;
    lg.add_attribute("ServiceType", attrs::constant<std::string>("RPC"));
    lg.add_attribute("ServerId", attrs::constant<std::string>(id));
}