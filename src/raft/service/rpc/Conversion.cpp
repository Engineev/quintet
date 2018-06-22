#include "service/rpc/Conversion.h"

// Common -> Pb
namespace quintet {
namespace rpc {

PbReply convertReply(const Reply &reply) {
  PbReply res;
  res.set_term(reply.first);
  res.set_ans(reply.second);
  return res;
}
PbServerId convertServerId(const ServerId &serverId) {
  PbServerId res;
  res.set_addr(serverId.addr);
  res.set_port(serverId.port);

  return res;
}
PbLogEntry convertLogEntry(const LogEntry &entry) {
  PbLogEntry res;
  res.set_term(entry.term);
  res.set_opname(entry.opName);
  res.set_args(entry.args);
  res.set_prmidx(entry.prmIdx);
  *res.mutable_srvid() = convertServerId(entry.srvId);
  return res;
}
PbAppendEntriesMessage
convertAppendEntriesMessage(const AppendEntriesMessage &msg) {
  PbAppendEntriesMessage res;
  res.set_term(msg.term);
  *res.mutable_leaderid() = convertServerId(msg.leaderId);
  res.set_prevlogidx(msg.prevLogIdx);
  res.set_prevlogterm(msg.prevLogTerm);
  for (auto &entry : msg.logEntries)
    *res.add_logentries() = convertLogEntry(entry);
  res.set_commitidx(msg.commitIdx);
  return res;
}
PbRequestVoteMessage convertRequestVoteMessage(const RequestVoteMessage &msg) {
  PbRequestVoteMessage res;
  res.set_term(msg.term);
  *res.mutable_candidateid() = convertServerId(msg.candidateId);
  res.set_lastlogidx(msg.lastLogIdx);
  res.set_lastlogterm(msg.lastLogTerm);
  return res;
}
PbAddLogMessage convertAddLogMessage(const AddLogMessage &msg) {
  PbAddLogMessage res;
  res.set_opname(msg.opName);
  res.set_args(msg.args);
  res.set_prmidx(msg.prmIdx);
  *res.mutable_srvid() = convertServerId(msg.srvId);
  return res;
}
PbAddLogReply convertAddLogReply(const AddLogReply &reply) {
  PbAddLogReply res;
  res.set_success(reply.success);
  *res.mutable_leaderid() = convertServerId(reply.leaderId);
  return res;
}

} // namespace rpc
} // namespace quintet

// Pb -> Common
namespace quintet {
namespace rpc {

Reply convertReply(const PbReply &reply) { return {reply.term(), reply.ans()}; }
ServerId convertServerId(const PbServerId &serverId) {
  ServerId res;
  res.addr = serverId.addr();
  res.port = serverId.port();
  return res;
}
LogEntry convertLogEntry(const PbLogEntry &entry) {
  LogEntry res;
  res.term = entry.term();
  res.opName = entry.opname();
  res.args = entry.args();
  res.prmIdx = entry.prmidx();
  res.srvId = convertServerId(entry.srvid());
  return res;
}
AppendEntriesMessage
convertAppendEntriesMessage(const PbAppendEntriesMessage &msg) {
  AppendEntriesMessage res;
  res.term = msg.term();
  res.leaderId = convertServerId(msg.leaderid());
  res.prevLogIdx = msg.prevlogidx();
  res.prevLogTerm = msg.prevlogterm();
  for (int i = 0; i < msg.logentries_size(); ++i)
    res.logEntries.emplace_back(convertLogEntry(msg.logentries(i)));
  res.commitIdx = msg.commitidx();
  return res;
}
RequestVoteMessage convertRequestVoteMessage(const PbRequestVoteMessage &msg) {
  RequestVoteMessage res;
  res.term = msg.term();
  res.candidateId = convertServerId(msg.candidateid());
  res.lastLogIdx = msg.lastlogidx();
  res.lastLogTerm = msg.lastlogterm();
  return res;
}
AddLogMessage convertAddLogMessage(const PbAddLogMessage &msg) {
  AddLogMessage res;
  res.opName = msg.opname();
  res.args = msg.args();
  res.prmIdx = msg.prmidx();
  res.srvId = convertServerId(msg.srvid());
  return res;
}
AddLogReply convertAddLogReply(const PbAddLogReply &reply) {
  AddLogReply res;
  res.success = reply.success();
  res.leaderId = convertServerId(reply.leaderid());
  return res;
}

} // namespace rpc
} // namespace quintet