#include "Server.h"

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
}

void quintet::Server::initService() {
    service.identityTransformer.bind([&](ServerIdentityNo to) {transform(to);});

    service.rpcService.listen(info.local.port);
    service.rpcService.bind("AppendEntries",
                            [&](Term term, ServerId leaderId,
                                std::size_t prevLogIdx, Term prevLogTerm,
                                std::vector<LogEntry> logEntries, std::size_t commitIdx) {
                                return RPCAppendEntries(term, leaderId,  prevLogIdx, prevLogTerm,
                                                        std::move(logEntries), commitIdx);
                            });
    service.rpcService.bind("RequestVote",
                            [&](Term term, ServerId candidateId,
                                std::size_t lastLogIdx, Term lastLogTerm) {
                                return RPCRequestVote(term, candidateId, lastLogIdx, lastLogTerm);
                            });

    service.logger.set("./", info.local.addr + "_" + std::to_string(info.local.port));
}

void quintet::Server::transform(quintet::ServerIdentityNo to) {
    auto from = currentIdentity;
#ifdef IDENTITY_TEST
    currentIdentity = from;
#else
    currentIdentity = to;
#endif

    service.rpcService.pause();
    service.heartBeatController.stop();

    if (from != ServerIdentityNo::Down)
        identities[(std::size_t)from]->leave();

    refreshState();

    if (to != ServerIdentityNo::Down)
        identities[(std::size_t)to]->init();

    service.heartBeatController.start();
    service.rpcService.resume();
}

void quintet::Server::refreshState() {
    state.votedFor = NullServerId;
}
