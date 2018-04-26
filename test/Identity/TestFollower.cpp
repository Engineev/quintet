#include <boost/test/unit_test.hpp>

#include <atomic>

#include <boost/thread.hpp>

#include "IdentityTestHelper.h"

BOOST_AUTO_TEST_SUITE(Identity)
BOOST_FIXTURE_TEST_SUITE(Follower, quintet::test::IdentityTestHelper)

BOOST_AUTO_TEST_CASE(StartElection) {
    BOOST_TEST_MESSAGE("Test::Identity::Follower::StartElection");

    const std::size_t SrvNum = 3;
    auto srvs = makeServers(SrvNum);
    const auto ElectionTimeout = srvs.front()->getElectionTimeout();

    std::atomic<int> follower2Candidate{0}; // count: follower -> candidate
    for (auto &srv : srvs) {
        srv->setBeforeTransform([&](No from, No to) {
            if (from == No::Down) return No::Follower;
            if (from == No::Follower && to == No::Candidate) {
                ++follower2Candidate;
                return No::Down;
            }
            if (from == No::Follower && to != No::Candidate) throw;
            return No::Down;

        });
        srv->setAfterTransform([&](No from, No to) {
            if (from == No::Candidate && to == No::Leader) {
                srv->sendHeartBeat();
            }
        });
        srv->run();
    }
    boost::this_thread::sleep_for(
        boost::chrono::milliseconds(ElectionTimeout * 5));
    for (auto &srv : srvs) {
        srv->stop();
    }
    BOOST_REQUIRE_EQUAL(follower2Candidate, SrvNum);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()