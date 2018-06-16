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
    Index nextIdx = Index(-1), matchIdx = 0;
  };
  std::unordered_map<ServerId, std::unique_ptr<FollowerState>> followerStates;
  std::unordered_map<Index, std::size_t> replicatedNum;

  EventQueue applyQueue;

  ThreadPool heartbeatThreads;

  void initFollowerStates();

  /// \breif Try indefinitely until succeed or \a nextIdx decrease to
  /// the lowest value.
  void tryAppendEntries(const ServerId & target, const AppendEntriesMessage & msg);

  Reply appendEntries(rpc::RpcClient &client) {} // TODO

  void init();

  void leave();

  Reply RPCAppendEntries(const AppendEntriesMessage &msg);

  Reply RPCRequestVote(const RequestVoteMessage &msg);

  void sendHeartBeats();

  void updateFollowerStatesAndReact() {}

  // helpers
  std::vector<rpc::RpcClient> makeRpcClients() const {
    std::vector<rpc::RpcClient> res;
    for (auto &srv : info.srvList) {
      if (srv == info.local)
        continue;
      res.emplace_back(
          grpc::CreateChannel(srv.addr + ":" + std::to_string(srv.port),
                              grpc::InsecureChannelCredentials()));
    }
    return res;
  }

  // commit and apply

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

void IdentityLeader::init() { pImpl->init(); }

void IdentityLeader::leave() { pImpl->leave(); }

Reply IdentityLeader::RPCAppendEntries(AppendEntriesMessage message) {
  return pImpl->RPCAppendEntries(message);
}

std::pair<Term /*current term*/, bool /*vote granted*/>
IdentityLeader::RPCRequestVote(RequestVoteMessage message) {
  return pImpl->RPCRequestVote(message);
}

} // namespace quintet

/* -------------------- Helper functions ------------------------------------ */

namespace quintet {

void IdentityLeader::Impl::init() {
  state.currentLeader = info.local;
  initFollowerStates();
  service.heartBeatController.bind(
      info.electionTimeout / 5,
      std::bind(&IdentityLeader::Impl::sendHeartBeats, this));
  service.heartBeatController.start(true, true);
}

void IdentityLeader::Impl::leave() {}

Reply IdentityLeader::Impl::RPCAppendEntries(const AppendEntriesMessage &msg) {}

Reply IdentityLeader::Impl::RPCRequestVote(const RequestVoteMessage &msg) {}

void IdentityLeader::Impl::initFollowerStates() {
  for (auto &srv : info.srvList) {
    if (srv == info.local)
      continue;
    auto tmp = std::make_unique<FollowerState>();
    tmp->nextIdx = state.entries.size();
    tmp->matchIdx = 0;
    followerStates.emplace(srv, std::move(tmp));
  }
}

void IdentityLeader::Impl::sendHeartBeats() {
  heartbeatThreads.clearWithInterruption();

  boost::shared_lock_guard<ServerState> slk(state);
  AppendEntriesMessage msg;
  msg.leaderId = info.local;
  msg.term = state.currentTerm;
  msg.commitIdx = state.commitIdx;
  msg.prevLogIdx = state.entries.size() - 2;
  msg.prevLogTerm = state.entries.at(msg.prevLogIdx).term;

  for (auto &srv : info.srvList) {
    if (srv == info.local)
      continue;
    heartbeatThreads.add(boost::thread([this, srv, msg] {
      rpc::RpcClient client(
          grpc::CreateChannel(srv.addr + ":" + std::to_string(srv.port),
                              grpc::InsecureChannelCredentials()));
      auto ctx = std::make_shared<grpc::ClientContext>();
      ctx->set_deadline(std::chrono::system_clock::now() +
                        std::chrono::milliseconds(50));
      auto res = client.callRpcAppendEntries(ctx, msg);
      if (res.first <= msg.term)
        return;
      boost::lock_guard<ServerState> sslk(state);
      state.currentTerm = res.first;
      service.identityTransformer.notify(ServerIdentityNo::Follower, msg.term);
      // TODO:
    }));
  }
}

} // namespace quintet

/* -------------------- tryAppendEntries ------------------------------------ */

namespace quintet {

void IdentityLeader::Impl::tryAppendEntries(const ServerId & target,
                                            const AppendEntriesMessage & msg) {
  rpc::RpcClient client(grpc::CreateChannel(
      target.toGrpcString(), grpc::InsecureChannelCredentials()));

  boost::strict_lock<ServerState> lk(state);
  auto & followerState = followerStates.at(target);
  boost::lock_guard<FollowerState> slk(*followerState);

  while (true) {
    auto ctx = std::make_shared<grpc::ClientContext>();
    ctx->set_deadline(std::chrono::system_clock::now() +
                      std::chrono::milliseconds(50));
    auto res = client.callRpcAppendEntries(ctx, msg);
    // TODO: exception
    if (res.second) {
      break;
    }
    if (res.first > state.currentTerm) {
      state.currentTerm = res.first;
      service.identityTransformer.notify(ServerIdentityNo::Follower,
                                         state.currentTerm);
      return;
    }
    followerState->nextIdx--;
    boost::this_thread::interruption_point();
  }

  // TODO: update matchIdx

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
  applyQueue.addEvent([entriesToApply = std::move(entriesToApply)] {
    for (auto &entry : entriesToApply) {
      service.apply(entry);
    }
  });
}

} // namespace quintet