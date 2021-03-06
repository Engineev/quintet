#include "identity/IdentityLeader.h"
#include <boost/test/unit_test.hpp>

#include <atomic>
#include <set>
#include <thread>
#include <mutex>

#include <grpc++/create_channel.h>

#include "IdentityTestHelper.h"
#include "RaftClient.h"
#include "service/rpc/RpcClient.h"

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(Identity)
BOOST_FIXTURE_TEST_SUITE(Leader, quintet::test::IdentityTestHelper)

BOOST_AUTO_TEST_CASE(Naive) {
  BOOST_TEST_MESSAGE("Test::Identity::Leader::Naive");
  using No = quintet::ServerIdentityNo;
  auto srvs = makeServers(1);
  std::unique_ptr<quintet::Raft> &srv = srvs.front();
  quintet::RaftDebugContext ctx;
  ctx.setBeforeTransform([](No from, No to) {
    if (from == No::Down && to == No::Follower)
      return No::Leader;
    if (to == No::Down)
      return No::Down;
    throw;
  });
  srv->setDebugContext(ctx);
  srv->AsyncRun();
  BOOST_REQUIRE_NO_THROW(srv->Stop());
}

BOOST_AUTO_TEST_CASE(Basic) {
  BOOST_TEST_MESSAGE("Test::Identity::Leader::Basic");
  using No = quintet::ServerIdentityNo;
  auto srvs = makeServers(1);
  std::unique_ptr<quintet::Raft> &srv = srvs.front();
  quintet::RaftDebugContext ctx;

  std::atomic<std::size_t> sendAppendEntriesTimes{0};
  ctx.setBeforeTransform([](No from, No to) {
       if (from == No::Down && to == No::Follower)
         return No::Leader;
       if (to == No::Down)
         return No::Down;
       throw;
     })
      .setBeforeSendRpcAppendEntries(
          [&sendAppendEntriesTimes](quintet::ServerId,
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

BOOST_AUTO_TEST_CASE(AddLog) {
  BOOST_TEST_MESSAGE("Test::Identity::Leader::AddLog");
  using No = quintet::ServerIdentityNo;
  using namespace quintet;

  auto srvs = makeServers(1);
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

  const std::size_t MsgN = 10;
  std::vector<AddLogMessage> msgs(MsgN);
  for (std::size_t i = 0; i < MsgN; ++i)
    msgs[i] = {"test", "", i, quintet::NullServerId};

  quintet::RaftClient client(leader->Local());
  std::vector<boost::future<AddLogReply>> replies;
  std::set<PrmIdx> ans;
  for (std::size_t i = 0; i < MsgN; ++i) {
    ans.insert(i);
    replies.emplace_back(
        client.asyncCallRpcAddLog(rpc::makeClientContext(50), msgs[i]));
  }
  for (auto &reply : replies)
    BOOST_REQUIRE(reply.get().success);

  const auto &state = leader->getState();
  const auto &entries = state.entries;
  std::set<PrmIdx> output;
  for (auto &entry : entries) {
    output.insert(entry.prmIdx);
  }

  BOOST_REQUIRE((ans == output));

  for (auto &srv : srvs)
    srv->Stop();
}

BOOST_AUTO_TEST_CASE(SyncBasic) {
  BOOST_TEST_MESSAGE("Test::Identity::Leader::Sync");
  using No = quintet::ServerIdentityNo;
  using namespace quintet;

  const std::size_t N = 1;
  auto srvs = makeServers(N);
  std::unique_ptr<quintet::Raft> &leader = srvs.front();
  std::mutex m;
  AppendEntriesMessage finalMsg;
  quintet::RaftDebugContext leaderCtx;
  leaderCtx
      .setBeforeTransform([](No from, No to) {
        if (from == No::Down && to == No::Follower)
          return No::Leader;
        if (to == No::Down)
          return No::Down;
        throw;
      })
      .setBeforeSendRpcAppendEntries(
          [&m, &finalMsg](ServerId, const AppendEntriesMessage &msg) {
            std::unique_lock<std::mutex> lk(m);
            finalMsg = msg;
          });
  leader->setDebugContext(leaderCtx);
  leader->AsyncRun();

  quintet::RaftClient client(leader->Local());
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

  std::set<PrmIdx> output;
  for (auto & entry : finalMsg.logEntries)
    output.insert(entry.prmIdx);
  BOOST_REQUIRE((ans == output));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()