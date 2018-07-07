#include "raft/rpc/conversion.h"
#include "rpc/conversion.h"

// Common -> Pb
namespace quintet {
namespace raft {
namespace rpc {

PbReply convertReply(const Reply &reply) {
  PbReply res;
  res.set_term(reply.get_term());
  res.set_ans(reply.get_success());
  return res;
}
PbLogEntry convertLogEntry(const LogEntry &entry) {
  PbLogEntry res;
  res.set_term(entry.get_term());
  res.set_opname(entry.get_opName());
  res.set_args(entry.get_args());
  res.set_prmidx(entry.get_prmIdx());
  return res;
}
PbAppendEntriesMessage
convertAppendEntriesMessage(const AppendEntriesMessage &msg) {
  PbAppendEntriesMessage res;
  res.set_term(msg.get_term());
  *res.mutable_leaderid() = quintet::rpc::convertServerId(msg.get_leaderId());
  res.set_prevlogidx(msg.get_prevLogIdx());
  res.set_prevlogterm(msg.get_prevLogTerm());
  for (auto &entry : msg.get_logEntries())
    *res.add_logentries() = convertLogEntry(entry);
  res.set_commitidx(msg.get_commitIdx());
  return res;
}
PbRequestVoteMessage convertRequestVoteMessage(const RequestVoteMessage &msg) {
  PbRequestVoteMessage res;
  res.set_term(msg.get_term());
  *res.mutable_candidateid() =
      quintet::rpc::convertServerId(msg.get_candidateId());
  res.set_lastlogidx(msg.get_lastLogIdx());
  res.set_lastlogterm(msg.get_lastLogTerm());
  return res;
}

} // namespace rpc
} // namespace raft
} // namespace quintet

// Pb -> Common
namespace quintet {
namespace raft {
namespace rpc {

Reply convertReply(const PbReply &reply) {
  return {reply.term(), reply.ans()};
}
LogEntry convertLogEntry(const PbLogEntry &entry) {
  return LogEntry(entry.opname(), entry.args(), entry.prmidx(), entry.term());
}
AppendEntriesMessage
convertAppendEntriesMessage(const PbAppendEntriesMessage &msg) {
  std::vector<LogEntry> entries;
  entries.reserve(msg.logentries_size());
  for (int i = 0; i < msg.logentries_size(); ++i)
    entries.emplace_back(convertLogEntry(msg.logentries(i)));
  return {msg.term(),         quintet::rpc::convertServerId(msg.leaderid()),
          msg.prevlogidx(),   msg.prevlogterm(),
          std::move(entries), msg.commitidx()};
}
RequestVoteMessage convertRequestVoteMessage(const PbRequestVoteMessage &msg) {
  return {msg.term(), quintet::rpc::convertServerId(msg.candidateid()),
          msg.lastlogidx(), msg.lastlogterm()};
}

} // namespace rpc
} // namespace raft
} // namespace quintet
