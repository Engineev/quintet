#include "Server.h"

#include <cassert>

void quintet::Server::init(const std::string &configDir) {
    info.load(configDir);
    initService();
    identities[(std::size_t)ServerIdentityNo::Follower]
            = std::make_unique<ServerIdentityFollower>(state, info, service);
    identities[(std::size_t)ServerIdentityNo::Candidate]
            = std::make_unique<ServerIdentityCandidate>(state, info, service);
    identities[(std::size_t)ServerIdentityNo::Leader]
            = std::make_unique<ServerIdentityLeader>(state, info, service);
    currentIdentity = ServerIdentityNo::Down;

}

void quintet::Server::run() {
    service.identityTransformer.transform(ServerIdentityNo::Follower);
}

void quintet::Server::stop() {
    service.identityTransformer.transform(ServerIdentityNo::Down);
    service.identityTransformer.stop();

    rpc.stop();
    service.heartBeatController.stop();
}

void quintet::Server::initService() {
    service.identityTransformer.bind([&](ServerIdentityNo to) { transform(to); });

    rpc.listen(info.local.port);
    rpc.bind("AppendEntries",
             [&](Term term, ServerId leaderId,
                 std::size_t prevLogIdx, Term prevLogTerm,
                 std::vector<LogEntry> logEntries, std::size_t commitIdx) {
                 return RPCAppendEntries(term, leaderId, prevLogIdx, prevLogTerm,
                                         std::move(logEntries), commitIdx);
             });
    rpc.bind("RequestVote",
             [&](Term term, ServerId candidateId,
                 std::size_t lastLogIdx, Term lastLogTerm) {
                 return RPCRequestVote(term, candidateId, lastLogIdx, lastLogTerm);
             });
    rpc.async_run();

    service.logger.set("./", info.local.addr + "_" + std::to_string(info.local.port));
}

void quintet::Server::transform(quintet::ServerIdentityNo to) {
#ifdef IDENTITY_TEST
    transform_test(to);
    return;
#endif
    auto from = currentIdentity;
    currentIdentity = to;

    rpc.pause();
    service.heartBeatController.stop();

    if (from != ServerIdentityNo::Down)
        identities[(std::size_t)from]->leave();

    refreshState();

    if (to != ServerIdentityNo::Down)
        identities[(std::size_t)to]->init();

    service.heartBeatController.start();
    rpc.resume();
}

void quintet::Server::refreshState() {
    state.votedFor = NullServerId;
}

std::pair<quintet::Term, bool>
quintet::Server::RPCRequestVote(quintet::Term term, quintet::ServerId candidateId, std::size_t lastLogIdx,
                                quintet::Term lastLogTerm) {
    if (currentIdentity == ServerIdentityNo::Down)
        return {-1, false};
    return identities[(int)currentIdentity]->RPCRequestVote(term, candidateId, lastLogIdx, lastLogTerm);
}

std::pair<quintet::Term, bool>
quintet::Server::RPCAppendEntries(quintet::Term term, quintet::ServerId leaderId, std::size_t prevLogIdx,
                                  quintet::Term prevLogTerm, std::vector<quintet::LogEntry> logEntries,
                                  std::size_t commitIdx) {
    return identities[(int)currentIdentity]->RPCAppendEntries(
            term, leaderId, prevLogIdx, prevLogTerm,
            std::move(logEntries), commitIdx);
}

void quintet::Server::bindCommit(std::function<void(LogEntry)> commit) {
    service.committer.bindCommit(std::move(commit));
}
