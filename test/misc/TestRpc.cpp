#include "service/rpc/RpcClient.h"
#include "service/rpc/RpcService.h"
#include <boost/test/unit_test.hpp>

#include <utility>
#include <thread>

#include <boost/atomic/atomic.hpp>

#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>

#include "service/rpc/RaftRpc.grpc.pb.h"
#include "service/rpc/Conversion.h"
#include "RaftClient.h"

namespace utf = boost::unit_test_framework;

BOOST_AUTO_TEST_SUITE(Service)
BOOST_AUTO_TEST_SUITE(Rpc)

BOOST_AUTO_TEST_CASE(ServerBaisc) {
  BOOST_TEST_MESSAGE("Service::Rpc::ServerBasic");
  using namespace quintet;
  using namespace quintet::rpc;

  const Port port = 50001;
  const ServerId srv("localhost", port);
  RpcService service;
  boost::atomic<int> appendEntriesCnt{0};
  service.bindAppendEntries([&appendEntriesCnt](AppendEntriesMessage) {
    ++appendEntriesCnt;
    return std::make_pair(Term(), false);
  });
  service.asyncRun(port);

  auto stub = RaftRpc::NewStub(
      grpc::CreateChannel(srv.addr + ":" + std::to_string(srv.port),
                          grpc::InsecureChannelCredentials()));

  auto call = [&stub] {
    grpc::ClientContext ctx;
    PbReply reply;
    AppendEntriesMessage msg;
    return stub->AppendEntries(&ctx, convertAppendEntriesMessage(msg), &reply);
  };

  for (int i = 1; i < 5; ++i) {
    auto status = call();
    BOOST_REQUIRE(status.ok());
    BOOST_REQUIRE_EQUAL(appendEntriesCnt, i);
  }

  service.stop();
}

BOOST_AUTO_TEST_CASE(ServerAdvanced) {
  BOOST_TEST_MESSAGE("Service::Rpc::ServerAdvanced");
  using namespace quintet;
  using namespace quintet::rpc;
  using namespace std::chrono_literals;

  const Port port = 50001;
  const ServerId srv("localhost", port);
  RpcService service;
  service.configLogger("RpcService");
  boost::atomic<int> cnt{0};
  service.bindRequestVote([&cnt](RequestVoteMessage) {
    ++cnt;
    return std::make_pair(Term(), false);
  });
  service.asyncRun(port);

  auto stub = RaftRpc::NewStub(
      grpc::CreateChannel(srv.addr + ":" + std::to_string(srv.port),
                          grpc::InsecureChannelCredentials()));
  auto fastCall = [&stub] {
    grpc::ClientContext ctx;
    PbReply reply;
    RequestVoteMessage msg;
    return stub->RequestVote(&ctx, convertRequestVoteMessage(msg), &reply);
  };

  service.pause();
  BOOST_TEST_CHECKPOINT("Paused");
  std::thread t([&fastCall] {
    auto status = fastCall();
    BOOST_REQUIRE(status.ok());
  });
  std::this_thread::sleep_for(10ms);
  BOOST_REQUIRE_EQUAL(0, cnt);
  BOOST_REQUIRE(t.joinable());
  BOOST_REQUIRE_NO_THROW(service.resume());
  t.join();
//  std::this_thread::sleep_for(10ms);
  BOOST_CHECK_EQUAL(1, cnt);
  BOOST_REQUIRE_NO_THROW(service.stop());
}

BOOST_AUTO_TEST_CASE(Basic) {
  BOOST_TEST_MESSAGE("Service::Rpc::Basic");
  using namespace quintet;
  using namespace quintet::rpc;

  const Port port = 50001;
  const ServerId srv("localhost", port);
  RpcService service;
  boost::atomic<int> cnt{0};
  service.bindAppendEntries([&cnt](AppendEntriesMessage) {
    ++cnt;
    return std::make_pair(Term(), false);
  });
  service.bindAddLog([&cnt](AddLogMessage) {
    ++cnt;
    return AddLogReply();
  });
  service.asyncRun(port);

  RpcClient client(grpc::CreateChannel(srv.addr + ":" + std::to_string(srv.port), grpc::InsecureChannelCredentials()));

  auto ctx = std::make_shared<grpc::ClientContext>();
  client.callRpcAppendEntries(std::move(ctx), {});
  BOOST_REQUIRE_EQUAL(1, cnt);
  RaftClient raftClient(srv);
  raftClient.callRpcAddLog(makeClientContext(50), {});
  BOOST_REQUIRE_EQUAL(2, cnt);

  BOOST_REQUIRE_NO_THROW(service.stop());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()