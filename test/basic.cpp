#include <boost/test/unit_test.hpp>
#include <quintet/server/server.h>

#include <thread>

BOOST_AUTO_TEST_CASE(Basic) {
  BOOST_TEST_MESSAGE("Test::Basic");
  quintet::Server srv(
      std::string(CMAKE_SOURCE_DIR) + "/test/config/raft0.json");
  srv.start();
  std::this_thread::sleep_for(std::chrono::seconds(3));
  srv.shutdown();
}