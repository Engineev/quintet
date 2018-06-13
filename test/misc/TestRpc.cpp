#include "service/rpc/RpcClients.h"
#include "service/rpc/RpcService.h"
#include <boost/test/unit_test.hpp>

#include <utility>
#include <thread>

#include <boost/atomic/atomic.hpp>

#include "service/rpc/RaftRpc.grpc.pb.h"
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>

namespace utf = boost::unit_test_framework;

BOOST_AUTO_TEST_SUITE(Service)
BOOST_AUTO_TEST_SUITE(Rpc)

BOOST_AUTO_TEST_CASE(ServerBaisc) {
  BOOST_TEST_MESSAGE("Service::Rpc::ServerBasic");
  using namespace quintet;
  using namespace quintet::rpc;

  const Port port = 50001;
  const ServerId srv = {.addr = "localhost", .port = port};
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
    PbAppendEntriesMessage msg;
    return stub->AppendEntries(&ctx, msg, &reply);
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
  const ServerId srv = {.addr = "localhost", .port = port};
  RpcService service;
  boost::atomic<int> cnt{0};
  service.bindAppendEntries([&cnt](AppendEntriesMessage) {
    std::this_thread::sleep_for(50ms);
    ++cnt;
    return std::make_pair(Term(), false);
  });
  service.bindRequestVote([&cnt](RequestVoteMessage) {
    ++cnt;
    return std::make_pair(Term(), false);
  });
  service.asyncRun(port);

  auto stub = RaftRpc::NewStub(
      grpc::CreateChannel(srv.addr + ":" + std::to_string(srv.port),
                          grpc::InsecureChannelCredentials()));
//  auto slowCall = [&stub] {
//    grpc::ClientContext ctx;
//    PbReply reply;
//    PbAppendEntriesMessage msg;
//    return stub->AppendEntries(&ctx, msg, &reply);
//  };
  auto fastCall = [&stub] {
    grpc::ClientContext ctx;
    PbReply reply;
    PbRequestVoteMessage msg;
    return stub->RequestVote(&ctx, msg, &reply);
  };

  service.pause();
  std::thread t([&fastCall] { fastCall(); });
  std::this_thread::sleep_for(10ms);
  BOOST_REQUIRE_EQUAL(0, cnt);
  service.resume();
  std::this_thread::sleep_for(2ms);
  BOOST_REQUIRE_EQUAL(1, cnt);
  service.stop();
  t.join();
}

BOOST_AUTO_TEST_CASE(Basic) {
  BOOST_TEST_MESSAGE("Service::Rpc::Basic");
  using namespace quintet;
  using namespace quintet::rpc;

  const Port port = 50001;
  const ServerId srv = {.addr = "localhost", .port = port};
  RpcService service;
  boost::atomic<int> cnt{0};
  service.bindAppendEntries([&cnt](AppendEntriesMessage) {
    ++cnt;
    return std::make_pair(Term(), false);
  });
  service.asyncRun(port);

  RpcClients clients;
  clients.createStubs({srv});
  clients.asyncRun();

  grpc::ClientContext ctx;
  clients.callRpcAppendEntries(srv, ctx, {});
  BOOST_REQUIRE_EQUAL(1, cnt);

  BOOST_REQUIRE_NO_THROW(service.stop());
  BOOST_REQUIRE_NO_THROW(clients.stop());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()