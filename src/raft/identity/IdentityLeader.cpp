#include "identity/IdentityLeader.h"

#include <memory>
#include <unordered_map>
#include <vector>

#include <boost/atomic/atomic.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/strict_lock.hpp>

#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>

#include "misc/EventQueue.h"
#include "misc/Rand.h"
#include "misc/Thread.h"
#include "service/log/Common.h"
#include "service/rpc/RpcClient.h"


/* -------------- constructors, destructors and Impl ------------------------ */

namespace quintet {

struct IdentityLeader::Impl : public IdentityBaseImpl {
  Impl(ServerState &state, ServerInfo &info, ServerService &service)
      : IdentityBaseImpl(state, info, service) {
    service.logger.add_attribute(
        "Part", logging::attrs::constant<std::string>("Identity"));
  }

  struct FollowerState
      : public boost::shared_lockable_adapter<boost::shared_mutex> {
    Index nextIdx = Index(-1);
    Index matchIdx = 0;
  };
  struct LogState : public boost::shared_lockable_adapter<boost::shared_mutex> {
    std::size_t replicateNum = 0;
  };

  std::unordered_map<ServerId, std::unique_ptr<FollowerState>> followerStates;
  std::unordered_map<Index, std::unique_ptr<LogState>> logStates;
  EventQueue applyQueue;

  // append entries
  Reply sendAppendEntries(boost::strict_lock<ServerState> &,
      rpc::RpcClient &client, Index start);

  /// \breif Try indefinitely until succeed or \a nextIdx decrease to
  /// the lowest value.
  void tryAppendEntries(const ServerId & target);

  // commit and reply
  /// \breif Update \a state.commitIdx and apply the newly-committed
  /// log entries asynchronously.
  ///
  /// This function is thread-safe andi t is guaranteed that no log
  /// entry will be applied more than once.
  ///
  /// \param commitIdx the new commitIdx
  void commitAndAsyncApply(boost::strict_lock<ServerState> &, Index commitIdx);


}; // struct IdentityLeader::Impl

IdentityLeader::IdentityLeader(ServerState &state, ServerInfo &info,
                               ServerService &service)
    : pImpl(std::make_unique<Impl>(state, info, service)) {}

IdentityLeader::~IdentityLeader() = default;

} // namespace quintet

/* --------------- public member functions & RPC ---------------------------- */

namespace quintet {

void IdentityLeader::init() { throw; }

void IdentityLeader::leave() { throw; }

Reply IdentityLeader::RPCAppendEntries(AppendEntriesMessage message) { throw; }

std::pair<Term /*current term*/, bool /*vote granted*/>
IdentityLeader::RPCRequestVote(RequestVoteMessage message) {
  throw;
}

} // namespace quintet

/* -------------------- AppendEntries --------------------------------------- */

namespace quintet {

// helper functions
namespace {

AppendEntriesMessage createAppendEntriesMessage(
    const ServerState & state, const ServerInfo & info,
    Index start) {
  AppendEntriesMessage msg;
  msg.term = state.currentTerm;
  msg.leaderId = info.local;
  msg.prevLogIdx = state.entries.size() - 2; // TODO: empty ?
  msg.prevLogTerm = state.entries[msg.prevLogIdx].term;
  msg.logEntries = std::vector<LogEntry>(
      state.entries.begin() + start, state.entries.end());
  msg.commitIdx = state.commitIdx;
  return msg;
}

auto timeout2Deadline(std::uint64_t ms) {
  return std::chrono::system_clock::now() + std::chrono::milliseconds(ms);
}

} // namespace

Reply IdentityLeader::Impl::sendAppendEntries(
    boost::strict_lock<ServerState> &, rpc::RpcClient &client, Index start) {
  auto msg = createAppendEntriesMessage(state, info, start);
  auto ctx = std::make_shared<grpc::ClientContext>();
  ctx->set_deadline(timeout2Deadline(50));
  return client.callRpcAppendEntries(ctx, msg);
}

void IdentityLeader::Impl::tryAppendEntries(const ServerId & target) {
  rpc::RpcClient client(grpc::CreateChannel(
      target.toString(), grpc::InsecureChannelCredentials()));

  boost::strict_lock<ServerState> serverStateLk(state);
  auto & followerState = followerStates.at(target);
  boost::unique_lock<FollowerState> followerStateLk(*followerState);

  while (true) {
    auto res = sendAppendEntries(serverStateLk, client, followerState->nextIdx);
    // TODO: exception; nextIdx == 0 ?
    if (res.second)
      break;
    if (res.first > state.currentTerm) {
      state.currentTerm = res.first;
      service.identityTransformer.notify(ServerIdentityNo::Follower,
                                         state.currentTerm);
      return;
    }
    followerState->nextIdx--;
    boost::this_thread::interruption_point();
  }

  // Succeed.
  followerState->matchIdx = state.entries.size() - 1;
  followerState->nextIdx = state.entries.size();
  followerStateLk.unlock();
  // TODO: update log states
}

} // namespace quintet

/* -------------------- Commit and Apply ------------------------------------ */

namespace quintet {

void IdentityLeader::Impl::commitAndAsyncApply(
    boost::strict_lock<ServerState> &, Index commitIdx) {
  if (state.commitIdx >= commitIdx)
    return;
  auto oldCommitIdx = state.commitIdx;
  state.commitIdx = commitIdx;

  std::vector<LogEntry> entriesToApply(state.entries.begin() + oldCommitIdx + 1,
                                       state.entries.begin() + commitIdx + 1);
  applyQueue.addEvent([this, entriesToApply = std::move(entriesToApply)] {
    for (std::size_t i = 0, sz = entriesToApply.size(); i < sz; ++i) {
      service.apply(entriesToApply[i]);
      boost::lock_guard<ServerState> lk(state);
      ++state.lastApplied; // TODO: assert
    }
  });
}

} // namespace quintet

