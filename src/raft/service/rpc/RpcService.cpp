#include "service/rpc/RpcService.h"

#include <functional>
#include <utility>
#include <vector>

#include "service/rpc/RaftRpc.grpc.pb.h"
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>

/* ---------------------------- RaftRpc ------------------------------------- */

namespace quintet {
namespace rpc {

class RaftRpcImpl final : public RaftRpc::Service {
  using AppendEntriesFunc = std::function<std::pair<Term, bool>(
      Term, ServerId, std::size_t, Term, std::vector<LogEntry>, std::size_t)>;
  using RequestVoteFunc =
      std::function<std::pair<Term, bool>(Term, ServerId, std::size_t, Term)>;

public:
  void bindAppendEntries(AppendEntriesFunc f) { appendEntries = std::move(f); }
  void bindRequestVote(RequestVoteFunc f) { requestVote = std::move(f); }

  grpc::Status AppendEntries(grpc::ServerContext *context,
                             const quintet::rpc::AppendEntriesMessage *request,
                             ::quintet::rpc::PbReply *response) override {
    Term leaderTerm = request->term();
    ServerId leaderId{request->leaderid().addr(),
                      (Port)request->leaderid().port()};
    std::size_t prevLogIdx = request->prevlogidx();
    Term prevLogTerm = request->prevlogterm();
    std::vector<LogEntry> logEntries;
    for (int i = 0; i < request->logentries_size(); ++i) {
      auto &pbEntry = request->logentries(i);
      LogEntry entry{pbEntry.term(), pbEntry.opname(), pbEntry.args(),
                     pbEntry.prmidx()};
      logEntries.emplace_back(entry);
    }
    std::size_t commitIdx = request->commitidx();

    Term currentTerm;
    bool success;
    std::tie(currentTerm, success) =
        appendEntries(leaderTerm, leaderId, prevLogIdx, prevLogTerm,
                      std::move(logEntries), commitIdx);
    response->set_term(currentTerm);
    response->set_ans(success);

    return grpc::Status::OK;
  }

  grpc::Status RequestVote(grpc::ServerContext *context,
                           const quintet::rpc::RequestVoteMessage *request,
                           quintet::rpc::PbReply *response) override {
    Term candidateTerm = request->term();
    ServerId candidateId{request->candidateid().addr(),
                         (Port)request->candidateid().port()};
    std::size_t lastLogIdx = request->lastlogidx();
    Term lastLogTerm = request->lastlogterm();

    Term currentTerm;
    bool granted;
    std::tie(currentTerm, granted) =
        requestVote(candidateTerm, candidateId, lastLogIdx, lastLogTerm);
    response->set_term(currentTerm);
    response->set_ans(granted);

    return grpc::Status::OK;
  }

private:
  AppendEntriesFunc appendEntries;
  RequestVoteFunc requestVote;
};

} // namespace rpc
} // namespace quintet

/* --------------------------- RpcService ----------------------------------- */

namespace quintet {
namespace rpc {

RpcService::RpcService() : pImpl(std::make_unique<Impl>()) {}

RpcService::~RpcService() = default;

struct RpcService::Impl {};

} // namespace rpc
} // namespace quintet