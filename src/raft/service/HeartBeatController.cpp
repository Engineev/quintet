#include "service/HeartBeatController.h"

#include <boost/chrono.hpp>
#include <boost/thread/locks.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/attributes/named_scope.hpp>

void quintet::HeartBeatController::stop() {
    BOOST_LOG(lg) << "HeartBeatController: stop()";
    th.interrupt();
    BOOST_LOG(lg) << "HeartBeatController: interrupted()";
    th.join();
    BOOST_LOG(lg) << "HeartBeatController: joined()";
}

void quintet::HeartBeatController::restart() {
    stop();
    start(immediateCache, repeatCache);
}

bool quintet::HeartBeatController::start(bool immediate, bool repeat) {
    BOOST_LOG_FUNCTION();
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
            BOOST_LOG(lg) << "HeartBeatController: beat!";
            f();
            if (!repeat)
                return ;
        }
        do {
            try {
                boost::this_thread::restore_interruption ri(di);
                boost::this_thread::sleep_for(boost::chrono::milliseconds(period));
            } catch (boost::thread_interrupted) {
                BOOST_LOG(lg) << "HeartBeatController: interrupted.";
                return;
            }
            BOOST_LOG(lg) << "HeartBeatController: beat!";
            f();
        } while (repeat);
    });
    return true;
}

void quintet::HeartBeatController::bind(std::function<void()> f, std::uint64_t periodMs) {
    func = std::move(f);
    period = periodMs;
}

quintet::HeartBeatController::~HeartBeatController() {
    BOOST_LOG_FUNCTION();
    stop();
    BOOST_LOG(lg) << "HeartBeatController: dtor";
}

void quintet::HeartBeatController::configLogger(const std::string &id) {
    using namespace logging;
    lg.add_attribute("ServerId", attrs::constant<std::string>(id));
}
