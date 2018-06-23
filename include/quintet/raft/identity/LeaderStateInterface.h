#ifndef QUINTET_LEADERSTATEINTERFACE_H
#define QUINTET_LEADERSTATEINTERFACE_H

#include <functional>
#include <memory>
#include <vector>

#include "StateInterface.h"
#include "ServerState.h"
#include "ServerInfo.h"
#include "service/rpc/RpcDefs.h"

namespace quintet {

// thread safe
class LeaderStateInterface : public StateInterface {
public:
  explicit LeaderStateInterface(ServerState & state);

  ~LeaderStateInterface() override ;

  const Term get_currentTerm() const;

  const Index get_commitIdx() const;

  const std::size_t entriesSize() const;

  std::vector<LogEntry> sliceLogEntries(Index beg, Index end);

  void incLastApplied();

  /// \breif currentTerm <- term if condition(currentTerm) is true
  ///
  /// If you want to get the original currentTerm, capture something in
  /// \a condition
  ///
  /// \param condition
  /// \param term The new term
  /// \return The result of condition(currentTerm), i.e whether \a currentTerm
  /// is updated
  bool set_currentTerm(std::function<bool(Term)> condition, Term term);

  bool set_commitIdx(std::function<bool(Index)> condition, Index idx);

  AppendEntriesMessage createAppendEntriesMessage(const ServerInfo & info,
                                                  Index start);

  AddLogReply addLog(const AddLogMessage & msg);

private:
  struct Impl;
  std::shared_ptr<Impl> pImpl;

}; // class LeaderStateInterface

} // namespace quintet

#endif //QUINTET_LEADERSTATEINTERFACE_H
