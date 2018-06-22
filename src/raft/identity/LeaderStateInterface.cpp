#include "identity/LeaderStateInterface.h"

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>

namespace quintet {

struct LeaderStateInterface::Impl {
  explicit Impl(ServerState & state) : state(state) {}

  ServerState & state;
  boost::shared_mutex currentTermM;
  boost::shared_mutex commitIdxM;
  boost::shared_mutex logEntriesM;
};

LeaderStateInterface::LeaderStateInterface(ServerState &state)
    : pImpl(std::make_unique<Impl>(state)) {}

LeaderStateInterface::~LeaderStateInterface() = default;

} // namespace quintet

namespace quintet {

AppendEntriesMessage LeaderStateInterface::createAppendEntriesMessage(
    const ServerInfo &info, Index start) {
  ServerState & state = pImpl->state;
  boost::shared_lock<boost::shared_mutex>
      commitIdxLk(pImpl->commitIdxM, boost::defer_lock),
      currentTermLk(pImpl->currentTermM, boost::defer_lock),
      logEntriesLk(pImpl->logEntriesM, boost::defer_lock);
  boost::lock(commitIdxLk, currentTermLk, logEntriesLk);

  AppendEntriesMessage msg;
  msg.term = state.get_currentTerm();
  msg.leaderId = info.local;
  assert(start > 0);
  msg.prevLogIdx = start - 1;
  msg.prevLogTerm = state.get_entries().at(msg.prevLogIdx).term;
  msg.logEntries = std::vector<LogEntry>(
      state.get_entries().begin() + start, state.get_entries().end());
  msg.commitIdx = state.get_commitIdx();

  return msg;
}

const Term LeaderStateInterface::get_currentTerm() const {
  boost::shared_lock_guard<boost::shared_mutex> lk(pImpl->currentTermM);
  return pImpl->state.get_currentTerm();
}

bool LeaderStateInterface::set_currentTerm(std::function<bool(Term)> condition,
                                           Term term) {
  auto & state = pImpl->state;
  boost::lock_guard<boost::shared_mutex> lk(pImpl->currentTermM);
  if (condition(state.get_currentTerm())) {
    state.getMutable_currentTerm() = term;
    return true;
  }
  return false;
}
const Index LeaderStateInterface::get_commitIdx() const {
  boost::shared_lock_guard<boost::shared_mutex> lk(pImpl->commitIdxM);
  return pImpl->state.get_commitIdx();
}

const std::size_t LeaderStateInterface::entriesSize() const {
  boost::shared_lock_guard<boost::shared_mutex> lk(pImpl->logEntriesM);
  return pImpl->state.get_entries().size();
}

AddLogReply LeaderStateInterface::addLog(const AddLogMessage &msg) {
  boost::shared_lock_guard<boost::shared_mutex> curTermLk(pImpl->currentTermM);
  boost::lock_guard<boost::shared_mutex> logEntriesLk(pImpl->logEntriesM);
  auto & state = pImpl->state;
  LogEntry log;
  log.term = state.get_currentTerm();
  log.prmIdx = msg.prmIdx;
  log.srvId = msg.srvId;
  log.args = msg.args;
  log.opName = msg.opName;
  state.getMutable_entries().emplace_back(std::move(log));
  return { true, NullServerId };
}

} // namespace quintet