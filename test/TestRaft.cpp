#include <boost/test/unit_test.hpp>
#include "Raft.h"

BOOST_AUTO_TEST_SUITE(Raft)

BOOST_AUTO_TEST_CASE(Basic) {
  BOOST_TEST_MESSAGE("Raft::Basic");
  quintet::Raft raft;
  raft.Configure(std::string(CMAKE_SOURCE_DIR) + "/test/RaftConfig/RaftConfig"
                     + std::to_string(0) + ".json");
  BOOST_REQUIRE_NO_THROW(raft.AsyncRun());
  BOOST_REQUIRE_NO_THROW(raft.Stop());
}

BOOST_AUTO_TEST_SUITE_END()