#ifndef QUINTET_ACTOR_TYPES_H
#define QUINTET_ACTOR_TYPES_H

#include <vector>

#include <theatre/all.h>

#include "common.h"
#include "common/client_context.h"
#include "raft_common.h"

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
using TimerBind = THEATRE_TAG("TimerBind");
using TimerStart = THEATRE_TAG("TimerStart");
using TimerStop = THEATRE_TAG("TimerStop");
using TimerRestart = THEATRE_TAG("TimerRe");

// RPC Sender
using SendAppendEntriesRpc = THEATRE_TAG("SAERpc");
using SendRequestVoteRpc = THEATRE_TAG("SRVRpc");
using ConfigSender = THEATRE_TAG("configSend");

// RPC Receiver
using BlockRpcReceiver = THEATRE_TAG("blockRpcR");
using UnblockRpcReceiver = THEATRE_TAG("unblockRpc");

// IdentityTransformer
using TriggerTransform = THEATRE_TAG("triTran");
using ResetTransformer = THEATRE_TAG("resetTran");

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

using TimerActor = theatre::Actor<
    theatre::Behavior<tag::TimerBind,
                      void(std::uint64_t /* timeout */, std::function<void()>)>,
    theatre::Behavior<tag::TimerStart,
                      void(bool /* immediate */, bool /* repeat */)>,
    theatre::Behavior<tag::TimerStop, void()>,
    theatre::Behavior<tag::TimerRestart, void()>>;

using RpcSenderActor = theatre::Actor<
    theatre::Behavior<tag::ConfigSender, void(const std::vector<ServerId> &)>,
    theatre::Behavior<
        tag::SendAppendEntriesRpc,
        AppendEntriesReply(ServerId, ClientContext, AppendEntriesMessage)>,
    theatre::Behavior<
        tag::SendRequestVoteRpc,
        RequestVoteReply(ServerId, ClientContext, RequestVoteMessage)>>;

using RpcReceiverActor = theatre::Actor<
    theatre::Behavior<tag::BlockRpcReceiver, void()>,
    theatre::Behavior<tag::UnblockRpcReceiver, void()>>;

using IdentityTransformerActor = theatre::Actor<
    theatre::Behavior<tag::TriggerTransform, void(IdentityNo /* target */)>,
    theatre::Behavior<tag::ResetTransformer, void()>>;

// clang-format on
} /* namespace quintet */

#endif // QUINTET_ACTOR_TYPES_H
