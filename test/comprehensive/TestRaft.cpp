#include <boost/test/unit_test.hpp>
#include "Raft.h"
#include "Server.h"

#include <atomic>
#include <thread>

#include "../identity/IdentityTestHelper.h"

namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE(Comprehensive, quintet::test::IdentityTestHelper,
                         *utf::disabled())

BOOST_AUTO_TEST_CASE(Basic) {
  BOOST_TEST_MESSAGE("Test::Comprehensive::Basic");
  using No = quintet::ServerIdentityNo;
  using namespace quintet;
  auto srvs = makeServers(3);
  RaftDebugContext ctx;
  std::atomic_size_t
      follower2Candidate{0},
      candidate2Leader{0},
      candidate2follower{0};
  ctx.setAfterTransform([&](No from, No to) {
    if (from == No::Down || to == No::Down)
      return to;
    if (from == No::Follower && to == No::Candidate) {
      ++follower2Candidate;
      return to;
    }
    if (from == No::Candidate && to == No::Leader) {
      ++candidate2Leader;
      return to;
    }
    if (from == No::Candidate && to == No::Follower) {
      ++candidate2follower;
      return to;
    }
    if (from == No::Candidate && to == No::Candidate)
      return to;
    throw ;
  });
  for (auto & srv : srvs) {
    srv->setDebugContext(ctx);
    srv->AsyncRun();
  }

  const auto ElectionTimeout = srvs[0]->getInfo().electionTimeout;
  std::this_thread::sleep_for(std::chrono::milliseconds(ElectionTimeout * 6));

  for (auto & srv : srvs)
    BOOST_REQUIRE_NO_THROW(srv->Stop());
  BOOST_REQUIRE_EQUAL(1, candidate2Leader);
  BOOST_REQUIRE_EQUAL(follower2Candidate - 1, candidate2follower);
}

BOOST_AUTO_TEST_SUITE_END()