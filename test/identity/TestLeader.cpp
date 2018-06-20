#include <boost/test/unit_test.hpp>
#include "identity/IdentityLeader.h"

#include <atomic>
#include <thread>

#include "IdentityTestHelper.h"

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(Identity)
BOOST_FIXTURE_TEST_SUITE(Leader, quintet::test::IdentityTestHelper)

BOOST_AUTO_TEST_CASE(Basic) {
  BOOST_TEST_MESSAGE("Test::Identity::Leader::Basic");
  using No = quintet::ServerIdentityNo;
  auto srvs = makeServers(2);
  std::unique_ptr<quintet::Raft> & srv = srvs.front();
  quintet::RaftDebugContext ctx;

  std::atomic<std::size_t> sendAppendEntriesTimes{0};
  ctx.setBeforeTransform([] (No from, No to) {
    if (from == No::Down && to == No::Follower)
      return No::Leader;
    if (to == No::Down)
      return No::Down;
    throw ;
  }).setBeforeSendRpcAppendEntries([&sendAppendEntriesTimes](quintet::ServerId,
                                      const quintet::AppendEntriesMessage &) {
    ++sendAppendEntriesTimes;
  });
  srv->setDebugContext(ctx);
  srv->AsyncRun();
  std::this_thread::sleep_for(
      std::chrono::milliseconds(srv->getInfo().electionTimeout * 3 / 2));
  srv->Stop();
  std::size_t otherSrvN = srv->getInfo().srvList.size() - 1;
  BOOST_REQUIRE((otherSrvN * 2 < sendAppendEntriesTimes));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()