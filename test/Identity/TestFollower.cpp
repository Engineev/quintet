#include <boost/test/unit_test.hpp>

#include <atomic>

#include <boost/thread.hpp>

#include "IdentityTestHelper.h"

BOOST_AUTO_TEST_SUITE(Identity)
BOOST_FIXTURE_TEST_SUITE(Follower, quintet::test::IdentityTestHelper)

BOOST_AUTO_TEST_CASE(StartElection, *boost::unit_test::disabled()) {
    BOOST_TEST_MESSAGE("Test::Identity::Follower::StartElection");

    const std::size_t SrvNum = 5;

    auto srvs = makeServers(SrvNum);

    const auto ElectionTimeout = srvs.front()->getElectionTimeout();
    std::atomic<int> transformPerformed{0}; // count: follower -> candidate
    for (auto & srv : srvs) {
        srv->setOnTransform([&](quintet::ServerIdentityNo from, quintet::ServerIdentityNo to) {
            if (from == quintet::ServerIdentityNo::Down)
                return to;
            if (from != quintet::ServerIdentityNo::Follower
                || to != quintet::ServerIdentityNo::Candidate)
                throw ;
            ++transformPerformed;
            return quintet::ServerIdentityNo::Down;
        });
    }
    runServers(srvs);
    boost::this_thread::sleep_for(boost::chrono::milliseconds(ElectionTimeout * 3));
    BOOST_REQUIRE_EQUAL(transformPerformed, SrvNum);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()