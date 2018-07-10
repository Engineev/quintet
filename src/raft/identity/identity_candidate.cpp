#include "raft/identity/identity_candidate.h"
#include "raft/identity/identity_base_impl.h"

#include <chrono>
#include <utility>

#include <boost/atomic.hpp>
#include <boost/thread/locks.hpp>

#include <grpc++/create_channel.h>
#include <grpc++/client_context.h>

#include "misc/rand.h"
//#include "service/log/Common.h"
#include "raft/rpc/rpc_client.h"
#include "rpc/error.h"
#include "client_context.h"


/* -------------------------------------------------------------------------- */

namespace quintet {
namespace raft {

struct IdentityCandidate::Impl : public IdentityBaseImpl {
  Impl(State &state, const ServerInfo &info, Service &service,
       const DebugContext &debugContext)
      : IdentityBaseImpl(state, info, service, debugContext) {
//    service.logger.add_attribute(
//        "Part", logging::attrs::constant<std::string>("Identity"));
  }

  boost::atomic<std::size_t> votesReceived{0};
  struct Request {
    std::unique_ptr<rpc::Client> client;
    ClientContext ctx;
    boost::thread t;

    Request() = default;
    Request(Request &&) = default;
  };
  std::vector<std::shared_ptr<Request>> requests;

  void requestVotes();

  RequestVoteMessage createRequestVoteMessage() const {
    Term term = state.syncGet_currentTerm();
    ServerId candidateId = info.get_local();
    boost::shared_lock_guard<boost::shared_mutex> lk(state.entriesM);
    Index lastLogIdx = state.entries.size() - 1;
    Term lastLogTerm = state.entries.back().get_term();
    return {term, candidateId, lastLogIdx, lastLogTerm};
  }
};

void IdentityCandidate::Impl::requestVotes() {
  for (auto &srv : info.get_srvList()) {
    if (srv == info.get_local())
      continue;
    RequestVoteMessage msg = createRequestVoteMessage();
    auto request = std::make_shared<Request>();
    request->ctx.setTimeout(50);
    request->client = std::make_unique<rpc::Client>(srv);
    request->t = boost::thread([this, request, msg, srv]() mutable {
      Term termReceived = InvalidTerm;
      bool res = false;

      for (int i = 0; i < 2; ++i) {
        try {
          auto reply = request->client->callRpcRequestVote(request->ctx, msg);
          termReceived = reply.get_term();
          res = reply.get_success();
        } catch (boost::thread_interrupted &e) {
          return;
        } catch (quintet::rpc::Error &e) {
          return;
        }
        break;
      }
      if (termReceived != msg.get_term() || !res)
        return;
      votesReceived += res;
      // It is guaranteed that only one transformation will be carried out.
      if (votesReceived > info.get_srvList().size() / 2) {
        service.identityTransformer.notify(IdentityNo::Leader);
      }
    });

    requests.emplace_back(std::move(request));
  }
}

} // namespace raft
} // namespace quintet

/* ---------------- public member functions --------------------------------- */

namespace quintet {
namespace raft {

IdentityCandidate::IdentityCandidate(
    State &state, const ServerInfo &info, Service &service,
    const DebugContext &context)
    : pImpl(std::make_unique<Impl>(state, info, service, context)) {}

IdentityCandidate::~IdentityCandidate() = default;

void IdentityCandidate::init() {
  pImpl->state.currentTerm++;
  pImpl->state.votedFor = pImpl->info.get_local();
  pImpl->votesReceived = 1;
  if (pImpl->votesReceived > pImpl->info.get_srvList().size() / 2) {
    pImpl->service.identityTransformer.notify(IdentityNo::Leader);
  }

  auto electionTimeout =
      intRand(pImpl->info.get_electionTimeout(),
              pImpl->info.get_electionTimeout()* 2);
//  BOOST_LOG(service.logger) << "ElectionTimeout = " << electionTimeout;

  std::vector<ServerId> srvs;
  std::copy_if(
      pImpl->info.get_srvList().begin(), pImpl->info.get_srvList().end(),
      std::back_inserter(srvs),
      [this](const ServerId &id) { return pImpl->info.get_local() != id; });


  pImpl->requestVotes();

  pImpl->service.heartbeatController.bind(electionTimeout, [this] {
        pImpl->service.identityTransformer.notify(IdentityNo::Candidate);
      });
  pImpl->service.heartbeatController.start(false, false);
}

void IdentityCandidate::leave() {
  pImpl->service.heartbeatController.stop();

//  for (auto &t : pImpl->requests)
//    t->ctx->TryCancel();
  for (auto &t : pImpl->requests)
    t->t.interrupt();
  for (auto &t : pImpl->requests)
    t->t.join();
  pImpl->requests.clear();
}

std::pair<bool, ServerId> IdentityCandidate::AddLog(BasicLogEntry entry) {
  return pImpl->defaultAddLog(std::move(entry));
}

} // namespace raft
} // namespace quintet

/* ---------------------- RPCs ---------------------------------------------- */

namespace quintet {
namespace raft {

Reply IdentityCandidate::RPCAppendEntries(AppendEntriesMessage msg) {
  int randId = intRand(100, 999);
//  BOOST_LOG(pImpl->service.logger)
//      << "{" << randId << "} Get RPCAppendEntries from "
//      << msg.leaderId.toString();
  Term curTerm = pImpl->state.syncGet_currentTerm();
  if (msg.get_term() == curTerm) {
//    BOOST_LOG(pImpl->service.logger) << "{" << randId << "} false. term ==";
    pImpl->service.identityTransformer.notify(IdentityNo::Follower);
  }
  return pImpl->defaultRPCAppendEntries(std::move(msg), IdentityNo::Candidate);
}

Reply IdentityCandidate::RPCRequestVote(RequestVoteMessage msg) {
  int randId = intRand(1000, 9999);
//  BOOST_LOG(pImpl->service.logger)
//      << "{" << randId << "} Get RPCRequestVote from "
//      << msg.candidateId.toString();
  return pImpl->defaultRPCRequestVote(std::move(msg),
                                      IdentityNo::Candidate, randId);
}

} // namespace raft
} // namespace quintet
