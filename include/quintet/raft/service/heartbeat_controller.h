#ifndef QUINTET_HEARTBEAT_CONTROLLER_H
#define QUINTET_HEARTBEAT_CONTROLLER_H

#include <cstdint>
#include <string>
#include <functional>
#include <memory>


namespace quintet {
namespace raft {

class HeartbeatController {
public:
  HeartbeatController();

  ~HeartbeatController();

  /// \breif Configure the logger. This function should be called before
  ///        other functions being called.
//  void configLogger(const std::string &id);

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
  struct Impl;
  std::unique_ptr<Impl> pImpl;

}; // class HeartbeatController


} // namespace raft
} // namespace quintet

#endif //QUINTET_HEARTBEAT_CONTROLLER_H
