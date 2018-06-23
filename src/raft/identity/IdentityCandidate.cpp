#include "identity/IdentityCandidate.h"
#include "identity/IdentityBaseImpl.h"

#include <chrono>
#include <utility>

#include <boost/atomic.hpp>
#include <boost/thread/locks.hpp>

#include <grpc++/create_channel.h>
#include <grpc++/client_context.h>

#include "misc/Rand.h"
#include "misc/Thread.h"
#include "service/log/Common.h"
#include "service/rpc/RpcClient.h"


/* -------------- constructors, destructors and Impl ------------------------ */

namespace quintet {

IdentityCandidate::~IdentityCandidate() = default;

struct IdentityCandidate::Impl : public IdentityBaseImpl {
  Impl(ServerState &state, const ServerInfo &info, ServerService &service,
    const RaftDebugContext & debugContext)
    : IdentityBaseImpl(state, info, service, debugContext) {
    service.logger.add_attribute(
      "Part", logging::attrs::constant<std::string>("Identity"));
  }

  boost::atomic<std::size_t> votesReceived{ 0 };
  struct Request {
    std::unique_ptr<rpc::RpcClient> client;
    std::shared_ptr<grpc::ClientContext> ctx;
    boost::thread t;

    Request() = default;
    Request(Request &&) = default;
  };
  std::vector<std::shared_ptr<Request>> requests;

  void init();

  void leave();

  void requestVotes();
};

IdentityCandidate::IdentityCandidate(
  ServerState &state, const ServerInfo &info,
  ServerService &service, const RaftDebugContext & context)
  : pImpl(std::make_unique<Impl>(state, info, service, context)) {}

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
  return pImpl->defaultRPCAppendEntries(std::move(msg));
}

Reply IdentityCandidate::RPCRequestVote(RequestVoteMessage msg) {
  return pImpl->defaultRPCRequestVote(std::move(msg));
}

AddLogReply IdentityCandidate::RPCAddLog(AddLogMessage message) {
  return pImpl->defaultAddLog(std::move(message));
}

} // namespace quintet

/* -------------------- Helper functions ------------------------------------ */

namespace quintet {

void IdentityCandidate::Impl::init() {
  state.currentTerm++;
  state.votedFor = info.local;

  auto electionTimeout =
    intRand(info.electionTimeout, info.electionTimeout * 2);
  BOOST_LOG(service.logger) << "ElectionTimeout = " << electionTimeout;

  std::vector<ServerId> srvs;
  std::copy_if(
    info.srvList.begin(), info.srvList.end(), std::back_inserter(srvs),
    [local = info.local](const ServerId &id) { return local != id; });

  votesReceived = 1;
  requestVotes();

  service.heartBeatController.bind(
    electionTimeout, [this] {
    auto term = state.get_currentTerm();
    service.identityTransformer.notify(ServerIdentityNo::Candidate, term);
  });
  service.heartBeatController.start(false, false);
}

void IdentityCandidate::Impl::leave() {
  BOOST_LOG(service.logger)
    << "leave(); " << requests.size() << " threads remaining";
  auto start = boost::chrono::high_resolution_clock::now();
  service.heartBeatController.stop();

  for (auto &t : requests)
    t->ctx->TryCancel();
  for (auto &t : requests)
    t->t.interrupt();
  for (auto &t : requests)
    t->t.join();
  requests.clear();
  BOOST_LOG(service.logger)
    << "leave()[done]; took "
    << boost::chrono::duration_cast<boost::chrono::milliseconds>(
      boost::chrono::high_resolution_clock::now() - start)
    .count()
    << " ms";
}

void IdentityCandidate::Impl::requestVotes() {
  for (auto &srv : info.srvList) {
    if (srv == info.local)
      continue;

    RequestVoteMessage msg;
    { // create the message
      msg.term = state.get_currentTerm();
      msg.candidateId = info.local;
      boost::shared_lock_guard<boost::shared_mutex> lk(state.entriesM);
      msg.lastLogIdx = state.entries.size() - 1;
      msg.lastLogTerm = state.entries.back().term;
    }

    auto request = std::make_shared<Request>();
    request->ctx = std::make_shared<grpc::ClientContext>();
    request->ctx->set_deadline(std::chrono::system_clock::now() +
      std::chrono::milliseconds(50));
    request->client = std::make_unique<rpc::RpcClient>(grpc::CreateChannel(
      srv.addr + ":" + std::to_string(srv.port),
      grpc::InsecureChannelCredentials()));
    request->t = boost::thread([this, request, msg, srv]() mutable {
      Term termReceived;
      bool res;
      auto tag = intRand(0, 100);

      for (int i = 0; i < 2; ++i) {
        try {
          BOOST_LOG(service.logger)
            << "{" << tag << "} Sending RequestVote to " << srv.toString();
          std::tie(termReceived, res) =
            request->client->callRpcRequestVote(request->ctx, msg);
        }
        catch (boost::thread_interrupted &e) {
          BOOST_LOG(service.logger) << "{" << tag << "} Interrupted";
          return;
        }
        catch (rpc::RpcError &e) {
          BOOST_LOG(service.logger)
            << "{" << tag << "} RpcError: " << e.what();
          return;
        }
        break;
      }
      BOOST_LOG(service.logger)
        << "{" << tag
        << "} Receive the result of RPCRequestVote from " << srv.toString()
        << ". TermReceived = " << termReceived << ", res = " << res;
      if (termReceived != msg.term || !res)
        return;
      votesReceived += res;
      // It is guaranteed that only one transformation will be carried out.
      if (votesReceived > info.srvList.size() / 2) {
        service.identityTransformer.notify(ServerIdentityNo::Leader, msg.term);
      }
    });

    requests.emplace_back(std::move(request));
  }
}

} // namespace quintet