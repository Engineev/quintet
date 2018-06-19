#include <boost/test/unit_test.hpp>
#include "Interface.h"

BOOST_AUTO_TEST_SUITE(Interface)

BOOST_AUTO_TEST_CASE(Basic) {
  BOOST_TEST_MESSAGE("Interface::Basic");
  quintet::Interface inf;
  inf.bind("add", [](int a, int b) { return a + b; });
  inf.Start();
  auto ans = inf.call("add", 1, 2);
  BOOST_REQUIRE_EQUAL(3, boost::any_cast<int>(ans));
  inf.Shutdown();
}

BOOST_AUTO_TEST_SUITE_END()