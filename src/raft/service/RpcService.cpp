#include "RpcService.h"

#include <thread>

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
    std::unique_lock<std::mutex> lk(stopping);
    modifiedNumRpcRemaining.wait(lk, [this] {
        return !numRpcRemaining;
    });
    if (srv)
        srv->stop();
}

void quintet::RpcService::configLogger(const std::string &id) {
    using namespace logging;
    lg.add_attribute("ServiceType", attrs::constant<std::string>("RPC"));
    lg.add_attribute("ServerId", attrs::constant<std::string>(id));
}

void quintet::RpcService::pause() {
    BOOST_LOG(lg) << "pause";
    paused.lock();
    boost::unique_lock<boost::shared_mutex> lk(rpcing);
}

void quintet::RpcService::resume() {
    BOOST_LOG(lg) << "resume";
    paused.unlock();
}
