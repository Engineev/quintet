#define BOOST_TEST_MODULE Test Quintet
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "log/global.h"
#include "server_info.h"

BOOST_AUTO_TEST_CASE(HelloWorld) {
  BOOST_TEST_MESSAGE("Test::Hello World!");
  BOOST_REQUIRE(true);
}

struct GlobalFixture {
  GlobalFixture() {
    auto & initializer = quintet::logging::Initializer::getInstance();
    quintet::ServerInfo info;
    info.load(std::string(CMAKE_SOURCE_DIR) + "/test/raft/config/raft_0.json");
    for (const quintet::ServerId & srv : info.get_srvList())
      initializer.addId(srv.toString());
    initializer.init();
  }

  ~GlobalFixture() = default;
};


BOOST_TEST_GLOBAL_FIXTURE(GlobalFixture);