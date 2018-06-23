#include "identity/LeaderStateInterface.h"
#include "identity/StateInterfaceImpl.h"

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>

namespace quintet {

struct LeaderStateInterface::Impl : public StateInterfaceImpl {
  explicit Impl(ServerState & state) : StateInterfaceImpl(state) {}

  boost::shared_mutex lastAppliedM;
  boost::shared_mutex commitIdxM;
  boost::shared_mutex logEntriesM;
};

LeaderStateInterface::LeaderStateInterface(ServerState &state)
    : pImpl(std::make_shared<Impl>(state)) {
  StateInterface::pImpl = pImpl;
}

LeaderStateInterface::~LeaderStateInterface() = default;

} // namespace quintet

namespace quintet {

AppendEntriesMessage LeaderStateInterface::createAppendEntriesMessage(
    const ServerInfo &info, Index start) {
  boost::shared_lock_guard<ServerState> LK(pImpl->state);
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
  return pImpl->get_currentTerm();
}

bool LeaderStateInterface::set_currentTerm(std::function<bool(Term)> condition,
                                           Term term) {
  return pImpl->set_currentTerm(std::move(condition), term);
}

bool LeaderStateInterface::set_commitIdx(std::function<bool(Index)> condition,
                                         Index idx) {
  boost::shared_lock_guard<ServerState> LK(pImpl->state);
  auto & state = pImpl->state;
  boost::lock_guard<boost::shared_mutex> lk(pImpl->commitIdxM);
  if (condition(state.get_commitIdx())) {
    state.getMutable_commitIdx() = idx;
    return true;
  }
  return false;
}

const Index LeaderStateInterface::get_commitIdx() const {
  boost::shared_lock_guard<ServerState> LK(pImpl->state);
  boost::shared_lock_guard<boost::shared_mutex> lk(pImpl->commitIdxM);
  return pImpl->state.get_commitIdx();
}

const std::size_t LeaderStateInterface::entriesSize() const {
  boost::shared_lock_guard<ServerState> LK(pImpl->state);
  boost::shared_lock_guard<boost::shared_mutex> lk(pImpl->logEntriesM);
  return pImpl->state.get_entries().size();
}

AddLogReply LeaderStateInterface::addLog(const AddLogMessage &msg) {
  boost::shared_lock_guard<ServerState> LK(pImpl->state);
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

std::vector<LogEntry> LeaderStateInterface::sliceLogEntries(Index beg,
                                                            Index end) {
  boost::shared_lock_guard<ServerState> LK(pImpl->state);
  boost::shared_lock_guard<boost::shared_mutex> lk(pImpl->logEntriesM);
  auto & state = pImpl->state;
  return std::vector<LogEntry>(
      state.get_entries().begin() + beg,
      state.get_entries().begin() + end);
}

void LeaderStateInterface::incLastApplied() {
  boost::shared_lock_guard<ServerState> LK(pImpl->state);
  boost::lock_guard<boost::shared_mutex> lk(pImpl->lastAppliedM);
  ++pImpl->state.getMutable_lastApplied();
}

} // namespace quintet