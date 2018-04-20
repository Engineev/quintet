#ifndef QUINTET_HEARTBEATCONTROLLER_H
#define QUINTET_HEARTBEATCONTROLLER_H

#include <functional>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/log/sources/logger.hpp>

#include "log/Common.h"

namespace quintet {

class HeartBeatController {
public:
    HeartBeatController() = default;

    ~HeartBeatController();

    /// \breif Configure the logger. This function should be called before
    ///        other functions being called.
    void configLogger(const std::string &id);

    void bind(std::function<void()> f, std::uint64_t periodMs);

    /// \brief After the invocation, the function bound will be
    ///        call every 'periodMs' ms. The specific behavior
    ///        depends on the parameters.
    ///
    /// \param immediate If 'immediate' == true, the function will
    ///                  be called right after the invocation of start().
    /// \param repeat  If 'repeat' == true, the function will be call
    ///                periodically. Otherwise, it will be called just
    ///                for one time.
    /// \return If it is already beating, return false. Otherwise return true.
    bool start(bool immediate = true, bool repeat = true);

    /// \brief Restart the heartBeat with the parameters same as the
    ///        last succeeded start.
    void restart();

    /// \brief Stop the heart beat. The beating thread will be interrupted.
    ///        User should not try to start and stop the heart beat
    ///        simultaneously.
    void stop();


private:
    boost::mutex m;
    std::function<void()> func;
    boost::thread th;
    std::uint64_t period;

private: // cached parameters
    bool immediateCache = false;
    bool repeatCache = false;

private: // logging
    boost::log::sources::logger_mt lg;


}; // class HeartBeatController

} /* namespace quintet */
#endif //QUINTET_HEARTBEATCONTROLLER_H
