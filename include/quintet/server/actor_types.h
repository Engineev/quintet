#ifndef QUINTET_ACTOR_TYPES_H
#define QUINTET_ACTOR_TYPES_H

#include <theatre/all.h>

#include "raft/common.h"
#include "common.h"

namespace quintet {

namespace tag {

// RaftActor
using AppendEntries = THEATRE_TAG("append");
using RequestVote = THEATRE_TAG("vote");
using AddLog = THEATRE_TAG("addLog");

// ConfigActor
using LoadConfig = THEATRE_TAG("lConfig");
using SaveConfig = THEATRE_TAG("sConfig");

} /* namespace tag */

using RaftActor = theatre::Actor<
    theatre::Action<tag::AppendEntries, RpcReply(AppendEntriesMessage, DebugId)>,
    theatre::Action<tag::RequestVote, RpcReply(RequestVoteMessage, DebugId)>,
    theatre::Action<tag::AddLog, void()> /* TODO */>;

using ConfigActor = theatre::Actor<
    theatre::Action<tag::LoadConfig, ServerInfo(std::string /* filename */)>,
    theatre::Action<tag::SaveConfig, void(ServerInfo, std::string )>>;

} /* namespace quintet */

#endif //QUINTET_ACTOR_TYPES_H
