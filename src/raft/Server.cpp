#include "Server.h"

#include <cassert>
#include <chrono>
#include <thread>
#include <boost/log/attributes/constant.hpp>

#include "log/Common.h"
#include "Utility.h"

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
    auto res = triggerTransformation(ServerIdentityNo::Follower, InvalidTerm);
    assert(res);
}

void quintet::Server::stop() {
    BOOST_LOG(service.logger) << "Server::stop()";
    rpc.stop();
    service.identityTransformer.stop();
    boost::lock_guard<boost::mutex> lk(transforming);
    transformThread.join();

    if (currentIdentity != ServerIdentityNo::Down)
        identities[(std::size_t)currentIdentity]->leave();
}

void quintet::Server::initService() {
    service.identityTransformer.bindNotificationSlot(
            [this](ServerIdentityNo to, Term term) { return triggerTransformation(to, term); });
    service.identityTransformer.configLogger(info.local.toString());
    service.identityTransformer.start();

    service.heartBeatController.configLogger(info.local.toString());

    service.logger.add_attribute("ServerId", logging::attrs::constant<std::string>(info.local.toString()));

    rpc.listen(info.local.port);
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
    rpc.async_run();
}

void quintet::Server::refreshState() {
    state.votedFor = NullServerId;
}

std::pair<quintet::Term, bool>
quintet::Server::RPCRequestVote(quintet::Term term, quintet::ServerId candidateId, std::size_t lastLogIdx,
                                quintet::Term lastLogTerm) {
    BOOST_LOG(service.logger) << "RPCRequestVote from " << candidateId.toString();
    if (rpcLatencyUb > 0) {
        auto time = Rand(rpcLatencyLb, rpcLatencyUb)();
        BOOST_LOG(service.logger) << "RPCLatency = " << time << " ms.";
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
    }
    if (currentIdentity == ServerIdentityNo::Down)
        return {-1, false};
    return identities[(int)currentIdentity]->RPCRequestVote(term, candidateId, lastLogIdx, lastLogTerm);
}

std::pair<quintet::Term, bool>
quintet::Server::RPCAppendEntries(quintet::Term term, quintet::ServerId leaderId, std::size_t prevLogIdx,
                                  quintet::Term prevLogTerm, std::vector<quintet::LogEntry> logEntries,
                                  std::size_t commitIdx) {
    BOOST_LOG(service.logger) << "RPCAppendEntries from " << leaderId.toString();
    return identities[(int)currentIdentity]->RPCAppendEntries(
            term, leaderId, prevLogIdx, prevLogTerm,
            std::move(logEntries), commitIdx);
}

void quintet::Server::bindCommit(std::function<void(quintet::LogEntry)> commit) {
    service.committer.bindCommit(std::move(commit));
}

void quintet::Server::transform(quintet::ServerIdentityNo target) {
    auto from = currentIdentity;
    auto actualTarget = target;
#ifdef IDENTITY_TEST
    actualTarget = beforeTransform(from, target);
#endif
    currentIdentity = actualTarget;

    BOOST_LOG(service.logger) << "transform from " << IdentityNames[(int)from]
                              << " to " << IdentityNames[(int)target]
                              << " (actually to " << IdentityNames[(int)actualTarget] << ")";

    rpc.pause();

    if (from != ServerIdentityNo::Down)
        identities[(std::size_t)from]->leave();

    refreshState();

    if (actualTarget != ServerIdentityNo::Down)
        identities[(std::size_t)actualTarget]->init();
#ifdef IDENTITY_TEST
    afterTransform(from, target);
#endif

    rpc.resume();
    BOOST_LOG(service.logger) << "Transformation completed.";
}

bool quintet::Server::triggerTransformation(quintet::ServerIdentityNo target, Term term) {
    boost::lock_guard<boost::mutex> lk(transforming);
    if (termTransformed != InvalidTerm && term <= termTransformed) {
        BOOST_LOG(service.logger) << "A transformation been triggered in this term.";
        return false;
    }

    termTransformed = term;
    BOOST_LOG(service.logger) << "Succeed to trigger a transformation.";
    transformThread.join();
    transformThread = boost::thread(
            [this, target] {
                transform(target);
            });
    return true;
}

void quintet::Server::setBeforeTransform(
        std::function<quintet::ServerIdentityNo(quintet::ServerIdentityNo, quintet::ServerIdentityNo)> f) {
    beforeTransform = std::move(f);
}

std::uint64_t quintet::Server::getElectionTimeout() const {
    return info.electionTimeout;
}

quintet::ServerIdentityNo quintet::Server::getCurrentIdentity() const {
    return currentIdentity;
}

void quintet::Server::setAfterTransform(
        std::function<void(quintet::ServerIdentityNo, quintet::ServerIdentityNo)> f) {
    afterTransform = std::move(f);
}

void quintet::Server::sendHeartBeat() {
    std::vector<boost::thread> ts;
    for (auto & srv : info.srvList) {
        if (srv == info.local)
            continue;
        ts.emplace_back(boost::thread([srv, currentTerm = state.currentTerm, this] {
            rpc::client c(srv.addr, srv.port);
            c.call("AppendEntries", currentTerm, info.local, 0, 0, std::vector<LogEntry>(), 0);
        }));
    }
    for (auto & t : ts)
        t.join();
}
