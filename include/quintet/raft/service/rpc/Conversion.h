#ifndef QUINTET_CONVERSION_H
#define QUINTET_CONVERSION_H

#include <utility>

#include "RaftRpc.pb.h"
#include "RpcDefs.h"

// Common -> Pb
namespace quintet {
namespace rpc {

PbReply convertReply(const Reply &reply);

PbServerId convertServerId(const ServerId &serverId);

PbLogEntry convertLogEntry(const LogEntry &entry);

PbAppendEntriesMessage
convertAppendEntriesMessage(const AppendEntriesMessage &msg);

PbRequestVoteMessage convertRequestVoteMessage(const RequestVoteMessage &msg);

PbAddLogMessage convertAddLogMessage(const AddLogMessage & msg);

PbAddLogReply convertAddLogReply(const AddLogReply & reply);

} // namespace rpc
} // namespace quintet

// Pb -> Common
namespace quintet {
namespace rpc {

Reply convertReply(const PbReply &reply);

ServerId convertServerId(const PbServerId &serverId);

LogEntry convertLogEntry(const PbLogEntry &entry);

AppendEntriesMessage
convertAppendEntriesMessage(const PbAppendEntriesMessage &msg);

RequestVoteMessage convertRequestVoteMessage(const PbRequestVoteMessage &msg);

AddLogMessage convertAddLogMessage(const PbAddLogMessage & msg);

AddLogReply convertAddLogReply(const PbAddLogReply & reply);

} // namespace rpc
} // namespace quintet

#endif // QUINTET_CONVERSION_H
