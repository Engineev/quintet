#include "identity/IdentityCandidate.h"

#include <utility>

#include <boost/atomic.hpp>

#include "misc/Rand.h"
#include "service/log/Common.h"

/* -------------- constructors, destructors and Impl ------------------------ */

namespace quintet {

IdentityCandidate::~IdentityCandidate() = default;

struct IdentityCandidate::Impl : public IdentityBase::IdentityBaseImpl {
  Impl(ServerState &state, ServerInfo &info, ServerService &service)
      : IdentityBaseImpl(state, info, service) {
    service.logger.add_attribute(
        "Part", logging::attrs::constant<std::string>("Identity"));
  }

  boost::atomic<std::size_t> votesReceived{0};
  std::vector<boost::thread> requestingThreads;

  void init();

  void leave();

  std::pair<Term, bool> sendRequestVote(const ServerId &target,
                                        const RequestVoteMessage &msg) {
    BOOST_LOG(service.logger)
      << "Sending RequestVote to " << target.toString();
    grpc::ClientContext ctx;
    return service.clients.callRpcRequestVote(target, ctx, msg);
  }

  void requestVotes();
};

IdentityCandidate::IdentityCandidate(ServerState &state, ServerInfo &info,
                                     ServerService &service)
    : pImpl(std::make_unique<Impl>(state, info, service)) {}

} // namespace quintet

/* ---------------- public member functions --------------------------------- */

namespace quintet {

void IdentityCandidate::init() { pImpl->init(); }

void IdentityCandidate::leave() { pImpl->leave(); }

} // namespace quintet

/* ---------------------- RPCs ---------------------------------------------- */

namespace quintet {

std::pair<Term /*current term*/, bool /*success*/>
IdentityCandidate::RPCAppendEntries(AppendEntriesMessage msg) {
  auto &service = pImpl->service;
  auto &state = pImpl->state;
  // TODO
  boost::lock_guard<ServerState> lk(state);
  if (msg.term >= state.currentTerm) {
    state.currentTerm = msg.term;
    service.identityTransformer.notify(ServerIdentityNo::Follower,
                                       state.currentTerm);
    return {state.currentTerm, false};
  }
  return {state.currentTerm, false};
}

std::pair<Term /*current term*/, bool /*vote granted*/>
IdentityCandidate::RPCRequestVote(RequestVoteMessage msg) {
  auto &state = pImpl->state;

  boost::lock_guard<ServerState> lk(state);

  if (msg.term < state.currentTerm) {
    return {state.currentTerm, false};
  }
  if (msg.term > state.currentTerm) {
    state.votedFor = NullServerId;
    state.currentTerm = msg.term;
    pImpl->service.identityTransformer.notify(ServerIdentityNo::Follower,
                                              msg.term);
    //        return {state.currentTerm, false};
  }

  if ((state.votedFor == NullServerId || state.votedFor == msg.candidateId) &&
      upToDate(state, msg.lastLogIdx, msg.lastLogTerm)) {
    state.votedFor = msg.candidateId;
    return {state.currentTerm, true};
  }

  return {state.currentTerm, false};
}

} // namespace quintet

/* -------------------- Helper functions ------------------------------------ */

namespace quintet {

void IdentityCandidate::Impl::init() {
  ++state.currentTerm;
  state.votedFor = info.local;

  auto electionTimeout =
      intRand(info.electionTimeout, info.electionTimeout * 2);
  BOOST_LOG(service.logger) << "ElectionTimeout = " << electionTimeout;

  votesReceived = 1;
  requestVotes();

  service.heartBeatController.bind(
      electionTimeout, [this, term = state.currentTerm] {
        service.identityTransformer.notify(ServerIdentityNo::Candidate, term);
      });
  service.heartBeatController.start(false, false);
}

void IdentityCandidate::Impl::leave() {
  BOOST_LOG(service.logger)
      << "leave(); " << requestingThreads.size() << " threads remaining";
  service.heartBeatController.stop();
  for (auto &t : requestingThreads)
    t.interrupt();
  for (auto &t : requestingThreads)
    t.join();
  requestingThreads.clear();
}

void IdentityCandidate::Impl::requestVotes() {
  for (auto &srv : info.srvList) {
    if (srv == info.local)
      continue;

    RequestVoteMessage msg;
    msg.term = state.currentTerm;
    msg.candidateId = info.local;
    msg.lastLogIdx = state.entries.size() - 1;
    msg.lastLogTerm = state.entries.back().term;
    auto t = boost::thread([this, srv, msg]() mutable {
      boost::this_thread::disable_interruption di;
      Term termReceived;
      bool res;
      try {
        boost::this_thread::restore_interruption ri(di);
        std::tie(termReceived, res) = sendRequestVote(srv, msg);
      } catch (boost::thread_interrupted &t) {
        return;
      } catch (rpc::RpcError & e) {
        BOOST_LOG(service.logger) << "RpcError";
        return;
      }
      if (termReceived != InvalidTerm) {
      }
      if (termReceived != msg.term || !res)
        return;
      votesReceived += res;
      // It is guaranteed that only one transformation will be carried out.
      if (votesReceived > info.srvList.size() / 2) {
        service.identityTransformer.notify(ServerIdentityNo::Leader, msg.term);
      }
    });

    requestingThreads.emplace_back(std::move(t));
  }
}

} // namespace quintet