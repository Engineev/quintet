#include <boost/test/unit_test.hpp>
#include "identity/IdentityFollower.h"

#include <thread>

#include "IdentityTestHelper.h"

namespace utf = boost::unit_test_framework;

BOOST_AUTO_TEST_SUITE(Identity)
BOOST_FIXTURE_TEST_SUITE(Follower, quintet::test::IdentityTestHelper)

BOOST_AUTO_TEST_CASE(Basic) {
  BOOST_TEST_MESSAGE("Test::Identity::Follower::Basic");
  using No = quintet::ServerIdentityNo;
  const std::size_t SrvNum = 1;
  auto srvs = makeServers(SrvNum);

  for (int i = 0; i < (int)srvs.size(); ++i) {
    auto &srv = srvs[i];
    srv->setBeforeTransform([](No from, No to) {
      return to == No::Down ? No::Down : No::Follower;
    });
    srv->AsyncRun();
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  for (auto &srv : srvs)
    BOOST_REQUIRE_NO_THROW(srv->Stop());
}

BOOST_AUTO_TEST_CASE(Naive) {
  BOOST_TEST_MESSAGE("Test::Identity::Follower::Naive");
  using No = quintet::ServerIdentityNo;

  const std::size_t SrvNum = 3;
  auto srvs = makeServers(SrvNum);
  const auto ElectionTimeout = srvs.front()->getInfo().electionTimeout;

  std::atomic<int> follower2Candidate{0};
  for (auto &srv : srvs) {
    srv->setBeforeTransform([&](No from, No to) {
      if (to == No::Down || to == No::Follower)
        return to;
      if (from == No::Follower && to == No::Candidate) {
        ++follower2Candidate;
        return No::Down;
      }
      throw;
    });
    srv->AsyncRun();
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(ElectionTimeout * 2));
  for (auto &srv : srvs)
    srv->Stop();
  BOOST_REQUIRE_EQUAL(follower2Candidate, SrvNum);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()