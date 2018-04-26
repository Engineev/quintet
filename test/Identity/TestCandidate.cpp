#include <boost/test/unit_test.hpp>
#include "Server.h"

#include <memory>
#include <vector>
#include <string>

#include "Identity/IdentityTestHelper.h"

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(Identity)
BOOST_FIXTURE_TEST_SUITE(Candidate, quintet::test::IdentityTestHelper, *utf::disabled())

BOOST_AUTO_TEST_CASE(Naive) {
    BOOST_TEST_MESSAGE("Test::Identity::Candidate::Naive");
    using No = quintet::ServerIdentityNo;

    const std::size_t SrvNum = 3;
    auto srvs = makeServers(SrvNum);
    const auto ElectionTimeout = srvs.front()->getElectionTimeout();

    std::atomic<int> candidate2Follower{0}, candidate2Leader{0};
    for (auto & srv : srvs) {
        srv->setBeforeTransform([&](No from, No to) {
            if (to == No::Down)
                return No::Down;

            if (from == No::Down && to == No::Follower)
                return No::Candidate;
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
        srv->setAfterTransform([&](No from, No to) {
            if (from == No::Candidate && to == No::Leader) {
                srv->sendHeartBeat();
            }
        });
        srv->run();
    }
    boost::this_thread::sleep_for(boost::chrono::milliseconds(ElectionTimeout * 15));
    for (auto & srv : srvs)
        srv->stop();
    BOOST_REQUIRE_EQUAL(candidate2Leader, 1);
    BOOST_REQUIRE_EQUAL(candidate2Follower, SrvNum - 1);
}

BOOST_AUTO_TEST_CASE(PoorNetwork) {
    BOOST_TEST_MESSAGE("Test::Identity::Candidate::PoorNetwork");
    using No = quintet::ServerIdentityNo;

    const std::size_t SrvNum = 3;
    auto srvs = makeServers(SrvNum);
    std::vector<int> times(SrvNum, 0);
    const auto ElectionTimeout = srvs.front()->getElectionTimeout();
    for (int i = 0; i < (int)srvs.size(); ++i) {
        auto & srv = srvs[i];
        srv->setBeforeTransform([&times, i](No from, No to) {
            times[i]++;
            if (times[i] >= 5)
                return No::Down;
            return No::Candidate;
        });
        srv->setAfterTransform([&](No from, No to) {
            if (from == No::Candidate && to == No::Leader) {
                srv->sendHeartBeat();
            }
        });
        srv->setRpcLatency(ElectionTimeout, ElectionTimeout * 2);
        srv->run();
    }
    boost::this_thread::sleep_for(boost::chrono::milliseconds(ElectionTimeout * 50));
    for (auto & srv : srvs)
        BOOST_REQUIRE_NO_THROW(srv->stop());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()