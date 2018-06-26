#define BOOST_TEST_MODULE Test Quintet
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "service/log/Global.h"
#include <boost/log/trivial.hpp>

BOOST_AUTO_TEST_CASE(HelloWorld) {
  BOOST_TEST_MESSAGE("Test::Hello World!");
  BOOST_REQUIRE(true);
}

struct GlobalFixture {
  GlobalFixture() {
    auto & initializer = quintet::logging::Initializer::getInstance();
    initializer.addId("127.0.0.1:50001"); // TODO
    initializer.addId("127.0.0.2:50002");
    initializer.addId("127.0.0.3:50003");
    initializer.init();
  }

  ~GlobalFixture() = default;
};


BOOST_TEST_GLOBAL_FIXTURE(GlobalFixture);