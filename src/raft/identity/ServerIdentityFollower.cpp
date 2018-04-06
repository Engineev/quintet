#include "ServerIdentityFollower.h"

#include <random>

quintet::ServerIdentityFollower::ServerIdentityFollower(quintet::ServerState &state_, quintet::ServerInfo &info_,
                                                        quintet::ServerService &service_)
        : ServerIdentityBase(state_, info_, service_) {}

void quintet::ServerIdentityFollower::init() {
    ++state.currentTerm;

    // launch Leader Checker
    std::default_random_engine eg;
    service.heartBeatController.bind([&]{
        service.identityTransformer.transform(ServerIdentityNo::Candidate);
    }, info.electionTimeout + std::uniform_int_distribution<std::uint64_t>(0, info.electionTimeout)(eg));
}

void quintet::ServerIdentityFollower::leave() {}
