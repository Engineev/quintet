#include <boost/test/unit_test.hpp>
#include "Interface.h"

#include "QuintetTestHelper.h"

namespace utf = boost::unit_test;

BOOST_FIXTURE_TEST_SUITE(Interface, quintet::test::QuintetTestHelper, *utf::disabled())

BOOST_AUTO_TEST_CASE(Basic) {
  BOOST_TEST_MESSAGE("Interface::Basic");
  using namespace quintet;
  auto infs = makeInf(3);
  for (auto & inf : infs) {
    inf.bind("add", [](int a, int b) { return a + b; });
    inf.Start();
  }
  auto ans = infs[0].call("add", 1, 2);
  BOOST_REQUIRE_EQUAL(3, boost::any_cast<int>(ans));

  for (auto & inf : infs)
    inf.Shutdown();
}

BOOST_AUTO_TEST_SUITE_END()