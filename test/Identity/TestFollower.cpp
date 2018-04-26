#include <boost/test/unit_test.hpp>

#include <atomic>

#include <boost/thread.hpp>

#include "IdentityTestHelper.h"
#include "QuintetConfig.h"

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(Identity)
BOOST_FIXTURE_TEST_SUITE(Follower, quintet::test::IdentityTestHelper)

BOOST_AUTO_TEST_CASE(StartElection) {
    BOOST_TEST_MESSAGE("Test::Identity::Follower::StartElection");

    const std::size_t SrvNum = SERVER_NUM;

    auto srvs = makeServers(SrvNum);
    const auto ElectionTimeout = srvs.front()->getElectionTimeout();
    std::atomic<int> transformPerformed{0}; // count: follower -> candidate
    for (auto & srv : srvs) {
        srv->setBeforeTransform([&](quintet::ServerIdentityNo from, quintet::ServerIdentityNo to) {
            if (from == quintet::ServerIdentityNo::Down)
                return to;
            if (to == quintet::ServerIdentityNo::Down)
                return to;
            if (from != quintet::ServerIdentityNo::Follower
                || to != quintet::ServerIdentityNo::Candidate)
                throw std::runtime_error("unknown transformation");
            ++transformPerformed;
            return quintet::ServerIdentityNo::Down;
        });
        srv->run();
    }
    boost::this_thread::sleep_for(boost::chrono::milliseconds(ElectionTimeout * 3));
    for (auto & srv : srvs)
        srv->stop();
    BOOST_REQUIRE_EQUAL(transformPerformed, SrvNum);
}

BOOST_AUTO_TEST_CASE(HeartBeat) {
    const std::size_t SrvNum = SERVER_NUM;
    auto srvs = makeServers(SrvNum);
    const auto ElectionTimeout = srvs.front()->getElectionTimeout();

    std::atomic<int> transform{0};
    for (auto & srv : srvs) {
        srv->setBeforeTransform([&](quintet::ServerIdentityNo from, quintet::ServerIdentityNo to) {
            if (from == quintet::ServerIdentityNo::Down || to == quintet::ServerIdentityNo::Down)
                return to;
            ++transform;
            return quintet::ServerIdentityNo::Follower;
        });
        srv->run();
    }

    boost::thread heart([&] {
        while (true) {
            try {
                boost::this_thread::sleep_for(boost::chrono::milliseconds(ElectionTimeout) / 5);
                for (auto & srv : srvs) {
                    rpc::client c(srv->getInfo().local.addr, srv->getInfo().local.port);
                    c.call("AppendEntries", 100000, quintet::ServerId(), 0, 0, std::vector<quintet::LogEntry>(), 0);
                }
            } catch (boost::thread_interrupted & e) {
                return;
            }
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(ElectionTimeout * 10));

    heart.interrupt();
    heart.join();
    for (auto & srv : srvs)
        srv->stop();
    BOOST_REQUIRE_EQUAL(0, transform);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()