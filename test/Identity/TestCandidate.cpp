#include <boost/test/unit_test.hpp>
#include "Server.h"

#include <memory>
#include <vector>
#include <string>

#include "Identity/IdentityTestHelper.h"

BOOST_AUTO_TEST_SUITE(Identity)
BOOST_FIXTURE_TEST_SUITE(Candidate, quintet::test::IdentityTestHelper)

BOOST_AUTO_TEST_CASE(Naive) {
    BOOST_TEST_MESSAGE("Test::Identity::Candidate::Naive");
    using No = quintet::ServerIdentityNo;

    const std::size_t SrvNum = 3;
    auto srvs = makeServers(SrvNum);
    const auto ElectionTimeout = srvs.front()->getElectionTimeout();

    std::atomic<int> candidate2Follower{0}, candidate2Leader{0};
    for (auto & srv : srvs) {
        srv->setOnTransform([&](No from, No to) {
            if (from == No::Down)
                return to;
            if (from == No::Candidate && to == No::Leader) {
                ++candidate2Leader;
                return No::Down;
            }
            if (from == No::Candidate && to == No::Follower) {
                ++candidate2Follower;
                return No::Down;
            }
            if (from == No::Candidate && to == No::Candidate)
                return No::Candidate;
            throw ;
        });
        srv->setIdentity_test(No::Candidate);
    }
    boost::this_thread::sleep_for(boost::chrono::milliseconds(ElectionTimeout * 30));
    for (auto & srv : srvs)
        srv->stop();
//    BOOST_REQUIRE_EQUAL(candidate2Leader, 1);
//    BOOST_REQUIRE_EQUAL(candidate2Follower, SrvNum - 1);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()