#include "raft/identity/identity_leader.h"
#include <boost/test/unit_test.hpp>

#include <atomic>
#include <set>
#include <thread>
#include <mutex>

#include "./identity_test_helper.h"
#include "raft/rpc/rpc_client.h"
#include "raft/debug_context.h"
#include "client.h"

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(Identity)
BOOST_FIXTURE_TEST_SUITE(Leader, quintet::test::IdentityTestHelper)

BOOST_AUTO_TEST_CASE(Naive) {
  BOOST_TEST_MESSAGE("Test::Raft::Leader::Naive");
  using No = quintet::raft::IdentityNo;
  auto srvs = makeServers(1);
  std::unique_ptr<quintet::raft::Raft> &srv = srvs.front();
  quintet::raft::DebugContext ctx;
  ctx.getMutable_beforeTrans() = [](No from, No to) {
    if (from == No::Down && to == No::Follower)
      return No::Leader;
    if (to == No::Down)
      return No::Down;
    throw;
  };
  srv->setDebugContext(ctx);
  srv->Start();
  BOOST_REQUIRE_NO_THROW(srv->Shutdown());
}

BOOST_AUTO_TEST_CASE(Basic) {
  BOOST_TEST_MESSAGE("Test::Raft::Leader::Basic");
  using No = quintet::raft::IdentityNo;
  auto srvs = makeServers(1);
  std::unique_ptr<quintet::raft::Raft> &srv = srvs.front();
  quintet::raft::DebugContext ctx;

  std::atomic<std::size_t> sendAppendEntriesTimes{0};
  ctx.getMutable_beforeTrans() = [](No from, No to) {
        if (from == No::Down && to == No::Follower)
          return No::Leader;
        if (to == No::Down)
          return No::Down;
        throw;
      };
  ctx.getMutable_beforeSendingRpcAppendEntries() =
      [&sendAppendEntriesTimes](quintet::ServerId,
                                const quintet::raft::AppendEntriesMessage &) {
      ++sendAppendEntriesTimes;
    };
  srv->setDebugContext(ctx);
  srv->Start();
  std::this_thread::sleep_for(
      std::chrono::milliseconds(srv->getInfo().get_electionTimeout() * 3 / 2));
  srv->Shutdown();
  std::size_t otherSrvN = srv->getInfo().get_srvList().size() - 1;
  BOOST_REQUIRE_LE(otherSrvN * 2, sendAppendEntriesTimes);
}

#ifdef false

BOOST_AUTO_TEST_CASE(AddLog) {
  BOOST_TEST_MESSAGE("Test::Identity::Leader::Call");
  using No = quintet::raft::IdentityNo;
  using namespace quintet::raft;

  auto srvs = makeServers(1);
  std::unique_ptr<Raft> &leader = srvs.front();
  DebugContext leaderCtx;
  leaderCtx.getMutable_beforeTrans() = [](No from, No to) {
    if (from == No::Down && to == No::Follower)
      return No::Leader;
    if (to == No::Down)
      return No::Down;
    throw;
  };

  leader->setDebugContext(leaderCtx);
  leader->Start();

  const std::size_t MsgN = 10;
  std::vector<quintet::BasicLogEntry> msgs(MsgN);
  for (std::size_t i = 0; i < MsgN; ++i)
    msgs[i] = {"test", "", i};

  quintet::Client client(leader->getInfo().get_local());
  std::vector<boost::future<AddLogReply>> replies;
  std::set<quintet::PrmIdx> ans;
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
    srv->Shutdown();
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
  leader->Start();

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
    srv->Shutdown();

  std::set<PrmIdx> output;
  for (auto & entry : finalMsg.logEntries)
    output.insert(entry.prmIdx);
  BOOST_REQUIRE((ans == output));
}

#endif

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()