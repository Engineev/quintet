#include "ServerService.h"

// IdentityTransformer

void quintet::IdentityTransformer::bind(std::function<void(ServerIdentityNo)> transform) {
    transform_ = std::move(transform);
}

void quintet::IdentityTransformer::transform(quintet::ServerIdentityNo target) {
    std::unique_lock<std::mutex> lk(transforming, std::defer_lock);
    if (!lk.try_lock())
        return;
    transform_(target);
}

// RpcService

void quintet::RpcService::listen(quintet::Port port) {
    if (srv)
        srv->stop();
    srv = std::make_unique<rpc::server>(port);
}

void quintet::RpcService::async_run(size_t worker) {
    srv->async_run(worker);
}

void quintet::RpcService::pause() {
    std::lock(rpcing, pausing);
    std::unique_lock<boost::shared_mutex> rpcLk(rpcing, std::adopt_lock);
    std::unique_lock<std::mutex> pauseLk(pausing, std::adopt_lock);
    paused = true;
}

void quintet::RpcService::resume() {
    std::lock_guard<std::mutex> pauseLk(pausing);
    paused = false;
    cv.notify_all();
}

// Logger

quintet::Logger::Logger(std::string dir_, std::string id_)
        : dir(std::move(dir_)), id(std::move(id_)) {
    auto tmp = dir + id + ".log";
    fout.open(dir + id + ".log");
}

void quintet::Logger::set(std::string dir_, std::string id_) {
    std::lock_guard<std::mutex> lk(logging);
    dir = std::move(dir_);
    id  = std::move(id_);
    fout.close();
    fout.open(dir + id + ".log");
}

quintet::Logger::~Logger() {
    fout.close();
}

void quintet::Logger::log_impl() {
    fout << std::endl;
}
