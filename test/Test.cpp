#define BOOST_TEST_MODULE Test Quintet
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <boost/log/trivial.hpp>

BOOST_AUTO_TEST_CASE(HelloWorld) {
  BOOST_TEST_MESSAGE("Test::Hello World!");
  BOOST_REQUIRE(true);
}
