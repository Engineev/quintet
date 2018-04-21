#define BOOST_TEST_MODULE Test Quintet
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "log/Global.h"
#include <boost/log/trivial.hpp>

BOOST_AUTO_TEST_CASE(HelloWorld) {
    BOOST_TEST_MESSAGE("Test: Hello World!");
    BOOST_LOG_TRIVIAL(trace) << "A trace severity message";
    BOOST_REQUIRE(true);
}

struct GlobalFixture {
    GlobalFixture() {
        auto & initializer = quintet::logging::Initializer::getInstance();
        initializer.addId("foo");
        initializer.init();
    }

    ~GlobalFixture() {}
};


BOOST_TEST_GLOBAL_FIXTURE(GlobalFixture);