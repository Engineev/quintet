#ifndef QUINTET_ACTOR_TYPES_H
#define QUINTET_ACTOR_TYPES_H

#include <vector>

#include <theatre/all.h>

#include "common.h"
#include "common/client_context.h"
#include "raft/common.h"

namespace quintet {

namespace tag {

// RaftActor
using AppendEntries = THEATRE_TAG("append");
using RequestVote = THEATRE_TAG("vote");
using AddLog = THEATRE_TAG("addLog");
using TransformIdentity = THEATRE_TAG("TransId");

// ConfigActor
using LoadConfig = THEATRE_TAG("lConfig");
using SaveConfig = THEATRE_TAG("sConfig");

// Timer
using BindFunc = THEATRE_TAG("BindFunc");

// RPC Sender
using SendAppendEntriesRpc = THEATRE_TAG("SAERpc");
using SendRequestVoteRpc = THEATRE_TAG("SRVRpc");
using ConfigSender = THEATRE_TAG("configSend");

} /* namespace tag */

// clang-format off

using RaftActor = theatre::Actor<
    theatre::Behavior<tag::AppendEntries,
                      AppendEntriesReply(AppendEntriesMessage, DebugId)>,
    theatre::Behavior<tag::RequestVote,
                      RequestVoteReply(RequestVoteMessage, DebugId)>,
    theatre::Behavior<tag::AddLog, void()>, /* TODO */
    theatre::Behavior<tag::TransformIdentity, void(IdentityNo)>>;

using ConfigActor = theatre::Actor<
    theatre::Behavior<tag::LoadConfig, ServerInfo(std::string /* filename */)>,
    theatre::Behavior<tag::SaveConfig, void(ServerInfo, std::string)>>;

using RpcSenderActor = theatre::Actor<
    theatre::Behavior<tag::ConfigSender, void(const std::vector<ServerId> &)>,
    theatre::Behavior<
        tag::SendAppendEntriesRpc,
        AppendEntriesReply(ServerId, ClientContext, AppendEntriesMessage)>,
    theatre::Behavior<
        tag::SendRequestVoteRpc,
        RequestVoteReply(ServerId, ClientContext, RequestVoteMessage)>>;

// clang-format on
} /* namespace quintet */

#endif // QUINTET_ACTOR_TYPES_H
