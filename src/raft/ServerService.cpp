#include "ServerService.h"
#include <thread>
#include <chrono>

// IdentityTransformer

void quintet::IdentityTransformer::bind(std::function<void(ServerIdentityNo)> transform) {
    transform_ = [transform](ServerIdentityNo to, std::unique_lock<std::mutex> && lk_) {
        std::unique_lock<std::mutex> lk = std::move(lk_);
        transform(to);
    };
}

bool quintet::IdentityTransformer::transform(quintet::ServerIdentityNo target) {
    std::unique_lock<std::mutex> lk(transforming, std::defer_lock);
    if (!lk.try_lock())
        return false;
    std::thread(transform_, target, std::move(lk)).detach();
    return true;
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

void quintet::RpcService::stop() {
    srv->stop();
}



void quintet::Committer::bindCommit(std::function<void(LogEntry)> f) {
    commit_ = std::move(f);
}

void quintet::Committer::commit(quintet::LogEntry log) {
    commit_(std::move(log));
}

void quintet::FaultInjector::randomSleep(std::uint64_t lb, std::uint64_t ub) const {
#ifdef FAULT_INJECTION
    std::random_device rd;
    std::default_random_engine eg(rd());
    // use std::thread to disable interruption
    std::this_thread::sleep_for(std::chrono::milliseconds(
            std::uniform_int_distribution<std::uint64_t>(lb, ub)(eg)));
#endif
}
