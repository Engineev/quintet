#ifndef QUINTET_HEARTBEATCONTROLLER_H
#define QUINTET_HEARTBEATCONTROLLER_H

#include <cstdint>
#include <functional>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "service/log/Common.h"

namespace quintet {

class HeartBeatController {
public:
  HeartBeatController() = default;

  ~HeartBeatController();

  /// \breif Configure the logger. This function should be called before
  ///        other functions being called.
  void configLogger(const std::string &id);

  void bind(std::uint64_t periodMs, std::function<void()> f);

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
  std::function<void()> func = nullptr;
  boost::thread th;
  std::uint64_t period = 0;
  boost::log::sources::logger_mt logger;

private: // cached parameters
  bool immediateCache = false;
  bool repeatCache = false;

}; // class HeartBeatController

} /* namespace quintet */
#endif //QUINTET_HEARTBEATCONTROLLER_H
