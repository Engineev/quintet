#ifndef QUINTET_HEARTBEATCONTROLLER_H
#define QUINTET_HEARTBEATCONTROLLER_H

#include <functional>

#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/attributes/named_scope.hpp>

#include "log/Common.h"

namespace quintet {
namespace experimental {

class HeartBeatController {
public:
    HeartBeatController() = default;

    ~HeartBeatController() {
        BOOST_LOG_FUNCTION();
    }

    /// \breif Configure the logger. This function should be called before
    ///        other functions being called.
    void configLogger(const std::string &id) {
        using namespace logging;
        lg.add_attribute("ServerId", attrs::constant<std::string>(id));
        lg.add_attribute("ServiceType", attrs::constant<std::string>("HeartBeatController"));
    }

    void bindHeartBeat(std::function<void()> f, std::uint64_t periodMs) {
        beat.func = std::move(f);
        beat.period = periodMs;
    }

    void bindOneShot(std::function<void()> f, std::uint64_t periodMs) {
        oneShot.func = std::move(f);
        oneShot.period = periodMs;
    }

public:
    /// \brief Start the heart beat. Call beat.func every beat.period ms.
    ///
    /// \param immediate If 'immediate' == true, beat.func will be called once
    ///                  immediately after starting.
    /// \return If the heart beat has already been started, return false.
    ///         Otherwise return true.
    bool startHeartBeat(bool immediate = true) {
        BOOST_LOG_FUNCTION();
        boost::unique_lock<boost::mutex> lk(beat.m, boost::defer_lock);
        if (!lk.try_lock()) {
            BOOST_LOG(lg) << "Failed to start HeartBeat.";
            return false;
        }
        if (immediate)
            beat.func();
        oneShot.th = boost::thread([this, lk = std::move(lk), period = beat.period, f = beat.func] {
            while (true) {
                try {
                    boost::this_thread::sleep_for(boost::chrono::milliseconds(period));
                } catch (boost::thread_interrupted) {
                    BOOST_LOG(lg) << "Interrupted.";
                    return;
                }
                f();
            }
        });
        BOOST_LOG(lg) << "Succeed to start HeartBeat.";
        return true;
    }

    /// \brief Launch an one shot. Call oneShot.func after oneShot.period ms.
    ///        This function will return immediately.
    ///
    /// \return return whether the oneShot is launched successfully.
    ///         If an one shot has already been launched, the attempt to launch
    ///         will fail and return false.
    bool startOneShot() {
        boost::unique_lock<boost::mutex> lk(oneShot.m, boost::defer_lock);
        if (!lk.try_lock())
            return false;
        oneShot.th = boost::thread([lk = std::move(lk), period = oneShot.period, f = oneShot.func] {
            try {
                boost::this_thread::sleep_for(boost::chrono::milliseconds(period));
            } catch (boost::thread_interrupted) {
                return;
            }
            f();
        });
        return true;
    }

    /// \brief Stop the heart beat. The beating thread will be interrupted.
    ///        User should not try to start and stop the heart beat
    ///        simultaneously.
    void stopHeartBeat() {
        stopPackage(beat);
    }

    /// \brief Stop the one shot. If it has not been triggered, it will be
    ///        interrupted and return almost immediately. User should not
    ///        try to start and stop an one shot simultaneously.
    void stopOneShot() {
        stopPackage(oneShot);
    }

    /// \brief Stop the heart beat and the one shot. It is guaranteed that
    ///        the threads will exit before this function returns.
    void stop() {
        stopOneShot();
        stopHeartBeat();
    }

private:
    struct Package {
        boost::mutex m;
        std::function<void()> func;
        boost::thread th;
        std::uint64_t period;
    };

    Package beat, oneShot;

    void stopPackage(Package &p) {
        p.th.interrupt();
        p.th.join();
    }

private: // logging
    boost::log::sources::logger_mt lg;


}; // class HeartBeatController

}
} /* namespace quintet */
#endif //QUINTET_HEARTBEATCONTROLLER_H
