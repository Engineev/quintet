#include <boost/test/unit_test.hpp>
#include "identity/IdentityCandidate.h"


#include <algorithm>
#include <chrono>
#include <iterator>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "IdentityTestHelper.h"

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(Identity)
BOOST_FIXTURE_TEST_SUITE(Candidate, quintet::test::IdentityTestHelper)

BOOST_AUTO_TEST_CASE(Basic) {
  BOOST_TEST_MESSAGE("Test::Identity::Candidate::Basic");
  using No = quintet::ServerIdentityNo;
  const std::size_t SrvNum = 1;
  auto srvs = makeServers(SrvNum);

  BOOST_TEST_CHECKPOINT("Made servers");

  quintet::RaftDebugContext ctx;
  ctx.setBeforeTransform([](No from, No to) {
    return to == No::Down ? No::Down : No::Candidate;
  });
  for (int i = 0; i < (int)srvs.size(); ++i) {
    auto &srv = srvs[i];
    srv->setDebugContext(ctx);
    srv->AsyncRun();
    BOOST_TEST_CHECKPOINT("Server " + std::to_string(i) + " is running.");
  }

  BOOST_TEST_CHECKPOINT("All servers are running");

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  for (auto &srv : srvs)
    BOOST_REQUIRE_NO_THROW(srv->Stop());
  BOOST_TEST_CHECKPOINT("All servers have been stopped");
}

BOOST_AUTO_TEST_CASE(Naive) {
  BOOST_TEST_MESSAGE("Test::Identity::Candidate::Naive");
  using No = quintet::ServerIdentityNo;

  const std::size_t SrvNum = 3;
  auto srvs = makeServers(SrvNum);
  const auto ElectionTimeout = srvs.front()->getInfo().electionTimeout;

  std::atomic<int> candidate2Follower{0}, candidate2Leader{0};
  for (auto &srv : srvs) {
    quintet::RaftDebugContext ctx;
    ctx.setBeforeTransform([&](No from, No to) {
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
      throw;
    }).setAfterTransform([&](No from, No to) {
      if (from == No::Candidate && to == No::Leader) {
        sendHeartBeat(srv->getInfo().srvList, srv->getInfo().local,
                      srv->getCurrentTerm());
      }
    });
    srv->setDebugContext(ctx);
    srv->AsyncRun();
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(ElectionTimeout * 15));
  for (auto &srv : srvs)
    srv->Stop();
  BOOST_REQUIRE_EQUAL(candidate2Leader, 1);
  BOOST_REQUIRE_EQUAL(candidate2Follower, SrvNum - 1);
}

BOOST_AUTO_TEST_CASE(PoorNetwork) {
  BOOST_TEST_MESSAGE("Test::Identity::Candidate::PoorNetwork");
  using No = quintet::ServerIdentityNo;

  const std::size_t SrvNum = 3;
  auto srvs = makeServers(SrvNum);
  std::vector<int> times(SrvNum, 0);
  const auto ElectionTimeout = srvs.front()->getInfo().electionTimeout;
  for (int i = 0; i < (int)srvs.size(); ++i) {
    auto &srv = srvs[i];
    quintet::RaftDebugContext ctx;
    ctx.setBeforeTransform([&times, i](No from, No to) {
      times[i]++;
      if (times[i] >= 5)
        return No::Down;
      return No::Candidate;
    }).setAfterTransform([&](No from, No to) {
      if (from == No::Candidate && to == No::Leader) {
        sendHeartBeat(srv->getInfo().srvList, srv->getInfo().local,
                      srv->getCurrentTerm());
      }
    });
    srv->setDebugContext(ctx);
    srv->setRpcLatency(ElectionTimeout, ElectionTimeout * 2);
    srv->AsyncRun();
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(ElectionTimeout * 50));
  for (auto &srv : srvs)
    BOOST_REQUIRE_NO_THROW(srv->Stop());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()