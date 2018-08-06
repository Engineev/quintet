#include <server/logger.h>

namespace quintet {

Logger &Logger::instance() {
  static Logger inst;
  return inst;
}
void Logger::setPath(std::string path_) { path = std::move(path_); }

void Logger::clear() {
  std::lock_guard<boost::shared_mutex> lk(structureM);
  loggers.clear();
}

void Logger::addId(const std::string &id) {
  std::lock_guard<boost::shared_mutex> lk(structureM);
  auto node = std::make_unique<Node>();
  node->fout.open(path + id + ".log");
  loggers.emplace(id, std::move(node));
}

std::string Logger::fmtCurrentTime() {
  using namespace std::chrono;
  auto now = system_clock::now();
  auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 10000;
  auto timer = system_clock::to_time_t(now);
  std::tm bt = *std::localtime(&timer);
  std::ostringstream oss;
  oss << std::put_time(&bt, "%F %T");
  oss << '.' << std::setfill('0') << std::setw(4) << ms.count();
  return oss.str();
}

std::string Logger::fmtIdentity(IdentityNo id) {
  if (id == IdentityNo::Down)
    return "Down     ";
  if (id == IdentityNo::Follower)
    return "Follower ";
  if (id == IdentityNo::Candidate)
    return "Candidate";
  if (id == IdentityNo::Follower)
    return "Follower ";
  return "Unknown  ";
}

} // namespace quintet