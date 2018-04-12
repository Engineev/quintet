#include "ServerService.h"

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

void quintet::RpcService::setTimeout(std::int64_t value) {
    timeOut = value;
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

quintet::HeartBeatController::HeartBeatController(std::function<void()> f, std::uint64_t periodMs) {
    bind(std::move(f), periodMs);
}

quintet::HeartBeatController::~HeartBeatController() {
    stop();
}

void quintet::HeartBeatController::bind(std::function<void()> f, std::uint64_t periodMs_) {
    heartBeat = std::move(f);
    periodMs  = periodMs_;
}

void quintet::HeartBeatController::start() {
    if (!periodMs)
        return;
    if (!running.exchange(true))
        beat = boost::thread(&HeartBeatController::run, this);
}

void quintet::HeartBeatController::oneShot(std::function<void()> f, std::uint64_t periodMs) {
    std::unique_lock<std::mutex> lk(launching, std::defer_lock);
    if (!lk.try_lock())
        return;
    oneShots.emplace_back([f, periodMs] {
        try {
            std::this_thread::sleep_for(std::chrono::milliseconds(periodMs));
        } catch (boost::thread_interrupted) {
            return;
        }
        f();
    });
}

void quintet::HeartBeatController::stop() {
    std::lock_guard<std::mutex> lk(launching);

    beat.interrupt();
    beat.join();
    resetOneShots();
}

void quintet::HeartBeatController::run() {
    while (true) {
        heartBeat();
        try {
            boost::this_thread::sleep_for(boost::chrono::milliseconds(periodMs));
        } catch (boost::thread_interrupted ) {
            break;
        }
    }
    running = false;
}

void quintet::HeartBeatController::resetOneShots() {
    for (auto && t : oneShots)
        t.interrupt();
    for (auto && t : oneShots)
        t.join();
    oneShots.clear();
}


void quintet::Committer::bindCommit(std::function<void(LogEntry)> f) {
    commit_ = std::move(f);
}

void quintet::Committer::commit(quintet::LogEntry log) {
    commit_(std::move(log));
}
