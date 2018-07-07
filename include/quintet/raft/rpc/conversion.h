#pragma once

#include <string>

#include "raft.pb.h"
#include "./common.h"
#include "../raft_common.h"

// Common -> Pb
namespace quintet {
namespace raft {
namespace rpc {

PbReply convertReply(const Reply &reply);

PbLogEntry convertLogEntry(const LogEntry &entry);

PbAppendEntriesMessage
convertAppendEntriesMessage(const AppendEntriesMessage &msg);

PbRequestVoteMessage convertRequestVoteMessage(const RequestVoteMessage &msg);

} // namespace rpc
} // namespace raft
} // namespace quintet

// Pb -> Common
namespace quintet {
namespace raft {
namespace rpc {

Reply convertReply(const PbReply &reply);

LogEntry convertLogEntry(const PbLogEntry &entry);

AppendEntriesMessage
convertAppendEntriesMessage(const PbAppendEntriesMessage &msg);

RequestVoteMessage convertRequestVoteMessage(const PbRequestVoteMessage &msg);


} // namespace rpc
} // namespace raft
} // namespace quintet