#include "ServerIdentityFollower.h"

#include <random>
#include <ctime>

quintet::ServerIdentityFollower::ServerIdentityFollower(quintet::ServerState &state_, quintet::ServerInfo &info_,
                                                        quintet::ServerService &service_)
        : ServerIdentityBase(state_, info_, service_) {}

void quintet::ServerIdentityFollower::init() {
    ++state.currentTerm;

    // launch Leader Checker
    std::random_device rd;
    std::default_random_engine eg(rd());
    auto electionTimeout = info.electionTimeout + std::uniform_int_distribution<std::uint64_t>(0, info.electionTimeout)(eg);

    service.logger("\n\tFollower::init()\n\telectionTimeout = ", electionTimeout);

    service.heartBeatController.bind([&]{
        service.identityTransformer.transform(ServerIdentityNo::Candidate);
    }, electionTimeout);
}

void quintet::ServerIdentityFollower::leave() {}
