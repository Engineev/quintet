#include <boost/test/unit_test.hpp>
//#include <altivec.h>
#include "misc/FileIO.h"

namespace utf = boost::unit_test_framework;

BOOST_AUTO_TEST_SUITE(Misc)
BOOST_AUTO_TEST_SUITE(FileIO)

BOOST_AUTO_TEST_CASE(Basic) {
  BOOST_TEST_MESSAGE("Misc::FileIO::Value");
  quintet::File file;
  file.registerVar<int>("valueTest", quintet::File::FILE_VALUE);
  file.modify<int>("valueTest", 100);
  file.modify<int>("valueTest", 10);
  int result = file.getValue<int>("valueTest");
  BOOST_REQUIRE_EQUAL(10, result);
}

BOOST_AUTO_TEST_CASE(Advanced) {
  BOOST_TEST_MESSAGE("Misc::FileIO::Vector");
  quintet::File file;
  file.registerVar<int>("vectorTest", quintet::File::FILE_VECTOR);
  file.clearFile<int>("vectorTest");
  file.append<int>("vectorTest", 10);
  file.append<int>("vectorTest", 100);
  std::vector<int> vec = file.getVector<int>("vectorTest");
  BOOST_REQUIRE_EQUAL(2, vec.size());
  BOOST_REQUIRE_EQUAL(10, vec[0]);
  BOOST_REQUIRE_EQUAL(100, vec[1]);

  file.pop<int>("vectorTest");
  vec = file.getVector<int>("vectorTest");
  BOOST_REQUIRE_EQUAL(1, vec.size());
  BOOST_REQUIRE_EQUAL(10, vec[0]);

  file.pop<int>("vectorTest");
  vec = file.getVector<int>("vectorTest");
  BOOST_REQUIRE_EQUAL(0, vec.size());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()