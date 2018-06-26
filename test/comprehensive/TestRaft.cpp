#include <boost/test/unit_test.hpp>
#include "Raft.h"
#include "Server.h"

#include <atomic>
#include <thread>

#include "RaftClient.h"
#include "service/rpc/RpcClient.h"
#include "../identity/IdentityTestHelper.h"

namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE(Comprehensive, quintet::test::IdentityTestHelper)

BOOST_AUTO_TEST_CASE(Election) {
  BOOST_TEST_MESSAGE("Test::Comprehensive::Election");
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
//    return to;
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

BOOST_AUTO_TEST_CASE(AppendEntries) {
  BOOST_TEST_MESSAGE("Test::Comprehensive::AppendEntries");
  using No = quintet::ServerIdentityNo;
  using namespace quintet;

  const std::size_t N = 3;
  auto srvs = makeServers(N);
  std::unique_ptr<quintet::Raft> &leader = srvs.front();
  quintet::RaftDebugContext leaderCtx;
  leaderCtx.setBeforeTransform([](No from, No to) {
    if (from == No::Down && to == No::Follower)
      return No::Leader;
    if (to == No::Down)
      return No::Down;
    throw;
  });
  leader->setDebugContext(leaderCtx);
  leader->AsyncRun();

  RaftDebugContext followerCtx;
  followerCtx.setBeforeTransform([](No from, No to) {
    if (to == No::Down || to == No::Follower)
      return to;
    throw;
  });
  for (std::size_t i = 1; i < N; ++i) {
    srvs[i]->setDebugContext(followerCtx);
    srvs[i]->AsyncRun();
  }

  RaftClient client(leader->Local());
  std::set<PrmIdx> ans;
  for (std::size_t i = 0; i < 10; ++i) {
    ans.insert(i);
    client.callRpcAddLog(rpc::makeClientContext(50),
                         {"test", "", i, quintet::NullServerId});
  }
  std::this_thread::sleep_for(
      std::chrono::milliseconds(leader->getInfo().electionTimeout * 10));

  for (auto &srv : srvs)
    srv->Stop();

  for (std::size_t i = 1; i < srvs.size(); ++i) {
    std::unique_ptr<quintet::Raft> & srv = srvs[i];
    auto & entries = srv->getState().entries;
    std::set<PrmIdx> output;
    for (auto &entry : entries)
      output.insert(entry.prmIdx);
    BOOST_REQUIRE((ans == output));
  }
}

BOOST_AUTO_TEST_SUITE_END()