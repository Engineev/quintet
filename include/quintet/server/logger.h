#ifndef QUINTET_LOGGER_H
#define QUINTET_LOGGER_H

#include <mutex>
#include <thread>
#include <fstream>
#include <unordered_map>
#include <iomanip>
#include <string>
#include <chrono>
#include <memory>

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/shared_lock_guard.hpp>

namespace quintet {

class Logger {
public:
  static Logger & instance();

  void setPath(std::string path_);

  void clear();

  void addId(const std::string & id);

  template <class... Msgs>
  void addLog(const std::string & id, const Msgs&... msgs) {
    boost::shared_lock_guard<boost::shared_mutex> structureLk(structureM);
    auto & node = *loggers.at(id);
    std::lock_guard<std::mutex> lk(node.writing);

    addLogImpl(node.fout, "[", fmtCurrentTime(), "] ", msgs...);
  }

private:
  Logger() = default;

  struct Node {
    std::ofstream fout;
    std::mutex writing;
  };
  boost::shared_mutex structureM;
  std::unordered_map<std::string, std::unique_ptr<Node>> loggers;
  std::string path = "./";

  void addLogImpl(std::ofstream & fout) {
    fout << std::endl;
  }

  template <class Msg, class... Msgs>
  void addLogImpl(std::ofstream & fout, const Msg & msg, const Msgs&... msgs) {
    fout << msg;
    addLogImpl(fout, msgs...);
  }

  std::string fmtCurrentTime();

}; // class Logger

} // namespace quintet

#endif //QUINTET_LOGGER_H
