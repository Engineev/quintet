#include <boost/test/unit_test.hpp>
#include "raft/ServerService.h"

#include <fstream>

BOOST_AUTO_TEST_SUITE(Service)

BOOST_AUTO_TEST_CASE(Logger, *boost::unit_test::disabled()) {
    BOOST_TEST_MESSAGE("Test::Logger");
    quintet::Logger log("./", "test");

    log();
    log(1, 2, 3);
    log("123");

    std::ifstream fin("./test.log");
    std::string str;

    getline(fin, str);
    BOOST_REQUIRE_EQUAL(str, "test: ");
    getline(fin, str);
    BOOST_REQUIRE_EQUAL(str, "test: 123");
    getline(fin, str);
    BOOST_REQUIRE_EQUAL(str, "test: 123");
} // case Logger

BOOST_AUTO_TEST_SUITE_END() // suite Service