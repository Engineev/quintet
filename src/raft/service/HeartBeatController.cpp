#include "service/HeartBeatController.h"

#include <boost/chrono.hpp>
#include <boost/thread/locks.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/attributes/named_scope.hpp>

void quintet::HeartBeatController::stop() {
    th.interrupt();
    th.join();
}

void quintet::HeartBeatController::restart() {
    stop();
    start(immediateCache, repeatCache);
}

bool quintet::HeartBeatController::start(bool immediate, bool repeat) {
    assert(period);

    boost::unique_lock<boost::mutex> lk(m, boost::defer_lock);
    if (!lk.try_lock()) {
        return false;
    }

    immediateCache = immediate;
    repeatCache = repeat;

    th = boost::thread(
            [this, lk = std::move(lk),
                    period = period, f = func,
                    immediate = immediate, repeat = repeat] {
        boost::this_thread::disable_interruption di;
        if (immediate) {
            f();
            if (!repeat)
                return ;
        }
        do {
            try {
                boost::this_thread::restore_interruption ri(di);
                boost::this_thread::sleep_for(boost::chrono::milliseconds(period));
            } catch (boost::thread_interrupted) {
                return;
            }
            f();
        } while (repeat);
    });
    return true;
}

void quintet::HeartBeatController::bind(
    std::function<void()> f, std::uint64_t periodMs) {
    func = std::move(f);
    period = periodMs;
}

quintet::HeartBeatController::~HeartBeatController() {
    stop();
}

void quintet::HeartBeatController::configLogger(const std::string &id) {
    using namespace logging;
    lg.add_attribute("ServiceType", attrs::constant<std::string>("HeartBeat"));
    lg.add_attribute("ServerId", attrs::constant<std::string>(id));
}
