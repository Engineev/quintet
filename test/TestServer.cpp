#include <boost/test/unit_test.hpp>
#include "Server.h"

BOOST_AUTO_TEST_SUITE(Server)

BOOST_AUTO_TEST_CASE(Init) {
    BOOST_TEST_MESSAGE("Test::Server::Init");
    quintet::Server srv;
    BOOST_REQUIRE_NO_THROW(
            srv.init(std::string(CMAKE_SOURCE_DIR) + "/test/RaftConfig/RaftConfig1.json"));
}

BOOST_AUTO_TEST_SUITE_END()