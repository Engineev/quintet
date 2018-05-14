#include "Server.h"

#include <array>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include "ServerIdentity.h"
#include "ServerInfo.h"
#include "ServerState.h"
#include "ServerService.h"
#include "RaftDefs.h"
#include "log/Common.h"

/* -------------- constructors, destructors and Impl ------------------------ */

namespace quintet {

Server::Server()
    : pImpl(std::make_unique<Impl>()) {}

Server::~Server() { stop(); };

struct Server::Impl {
    std::array<std::unique_ptr<ServerIdentityBase>, 3> identities;
    ServerIdentityNo currentIdentity = ServerIdentityNo::Down;

    ServerState   state;
    ServerInfo    info;
    ServerService service;
    RpcService    rpc;

    boost::mutex  triggerTransformation;
    Term          termTransformed = InvalidTerm;
    boost::thread transformThread;
};

} /* namespace quintet */

/* ---------------- public member functions --------------------------------- */

namespace quintet {

void Server::init(const std::string &configDir) {
    auto & state = pImpl->state;
    auto & info = pImpl->info;
    auto & service = pImpl->service;

    info.load(configDir);
    initServerService();
    pImpl->identities[(std::size_t)ServerIdentityNo::Follower]
        = std::make_unique<ServerIdentityFollower>(state, info, service);
    pImpl->identities[(std::size_t)ServerIdentityNo::Candidate]
        = std::make_unique<ServerIdentityCandidate>(state, info, service);
    pImpl->identities[(std::size_t)ServerIdentityNo::Leader]
        = std::make_unique<ServerIdentityLeader>(state, info, service);
    pImpl->currentIdentity = ServerIdentityNo::Down;
    initRpcServer();
}

void Server::bindCommit(std::function<void(LogEntry)> commit) {
    pImpl->service.committer.bindCommit(std::move(commit));
}

void Server::run() {
    auto res = triggerTransformation(ServerIdentityNo::Follower, InvalidTerm);
    assert(res);
}

void Server::stop() {
    BOOST_LOG(pImpl->service.logger) << "Server::stop()";

    pImpl->rpc.stop();
    pImpl->service.identityTransformer.stop();
    waitTransformation();

    if (pImpl->currentIdentity != ServerIdentityNo::Down)
        pImpl->identities[(int)pImpl->currentIdentity]->leave();

    pImpl->service.rpcClients.stop();
}


} /* namespace quintet */

/* ---------------------- RPCs ---------------------------------------------- */

namespace quintet {

std::pair<Term /*current term*/, bool /*success*/>
Server::RPCAppendEntries(Term term, ServerId leaderId, std::size_t prevLogIdx,
                         Term prevLogTerm, std::vector<LogEntry> logEntries,
                         std::size_t commitIdx) {
    BOOST_LOG_FUNCTION();
    BOOST_LOG(pImpl->service.logger)
        << "RPCAppendEntries from " << leaderId.toString();
    rpcSleep();

    if (pImpl->currentIdentity == ServerIdentityNo::Down)
        return {-1, false};
    return pImpl->identities[(int)pImpl->currentIdentity]->RPCAppendEntries(
        term, leaderId, prevLogIdx, prevLogTerm,
        std::move(logEntries), commitIdx);
}

std::pair<Term /*current term*/, bool /*vote granted*/>
Server::RPCRequestVote(Term term, ServerId candidateId,
                       std::size_t lastLogIdx, Term lastLogTerm) {
    BOOST_LOG_FUNCTION();
    BOOST_LOG(pImpl->service.logger)
        << "RPCRequestVote from " << candidateId.toString();
    rpcSleep();

    if (pImpl->currentIdentity == ServerIdentityNo::Down)
        return {-1, false};
    return pImpl->identities[(int)pImpl->currentIdentity]->RPCRequestVote(
        term, candidateId, lastLogIdx, lastLogTerm);
}



} /* namespace quintet */


/* ------------------ helper functions -------------------------------------- */

namespace quintet {

void Server::initServerService() {
    auto & service = pImpl->service;
    auto & info = pImpl->info;

    service.configLogger(info.local.toString());

    // identity transformer
    service.identityTransformer.bindNotificationSlot(
        [this](ServerIdentityNo to, Term term) {
        return triggerTransformation(to, term);
    });
    service.identityTransformer.start();

    // logger
    service.logger.add_attribute(
        "ServerId", logging::attrs::constant<std::string>(info.local.toString()));

    // rpc clients
    std::vector<ServerId> srvs;
    std::copy_if(info.srvList.begin(), info.srvList.end(), std::back_inserter(srvs),
                 [local = info.local] (const ServerId & id) {
        return local != id;
    });
    service.rpcClients.createClients(srvs);
}

bool Server::triggerTransformation(ServerIdentityNo target, Term term) {
    boost::lock_guard<boost::mutex> lk(pImpl->triggerTransformation);
    if (pImpl->termTransformed != InvalidTerm && term <= pImpl->termTransformed) {
        BOOST_LOG(pImpl->service.logger)
            << "A transformation been triggered in this term. "
            << "(" << term << " <= " << pImpl->termTransformed << ")";
        return false;
    }

    BOOST_LOG(pImpl->service.logger)
        << "Succeed to trigger a transformation. "
        << pImpl->termTransformed << " -> " << term;
    waitTransformation();
    pImpl->termTransformed = term;
    pImpl->transformThread = boost::thread([this, target] { transform(target); });
    return true;
}

void Server::transform(quintet::ServerIdentityNo target) {
    // leaving ...
    auto from = pImpl->currentIdentity;
    pImpl->rpc.pause();
    if (from != ServerIdentityNo::Down)
        pImpl->identities[(std::size_t)from]->leave();

    // transforming ...
    auto actualTarget = target;
#ifdef IDENTITY_TEST
    actualTarget = beforeTransform(from, target);
#endif
    BOOST_LOG(pImpl->service.logger)
        << "transform from " << IdentityNames[(int)from]
        << " to " << IdentityNames[(int)target]
        << " (actually to " << IdentityNames[(int)actualTarget] << ")";
    refreshState();
#ifdef IDENTITY_TEST
    afterTransform(from, target);
#endif

    // restart !
    if (actualTarget != ServerIdentityNo::Down)
        pImpl->identities[(int)actualTarget]->init();

    pImpl->rpc.resume();
    BOOST_LOG(pImpl->service.logger) << "Transformation completed.";
}

void Server::initRpcServer() {
    auto & rpc = pImpl->rpc;
    rpc.configLogger(pImpl->info.local.toString());
    rpc.listen(pImpl->info.local.port);
    rpc.bind("AppendEntries",
             [this](Term term, ServerId leaderId,
                    std::size_t prevLogIdx, Term prevLogTerm,
                    std::vector<LogEntry> logEntries, std::size_t commitIdx) {
                 return RPCAppendEntries(term, leaderId, prevLogIdx, prevLogTerm,
                                         std::move(logEntries), commitIdx);
             });
    rpc.bind("RequestVote",
             [this](Term term, ServerId candidateId,
                    std::size_t lastLogIdx, Term lastLogTerm) {
                 return RPCRequestVote(term, candidateId, lastLogIdx, lastLogTerm);
             });
    rpc.async_run(2);
}

void Server::refreshState() {
    pImpl->state.votedFor = NullServerId;
}


} /* namespace quintet */