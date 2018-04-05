#include <boost/test/unit_test.hpp>
#include "Interface.h"

#include <string>
#include <queue>
#include <thread>
#include <random>
#include <sstream>
#include <utility>
#include <chrono>
#include <vector>
#include <atomic>
#include <algorithm>
#include <thread>
#include <future>
#include <functional>

#include <boost/any.hpp>

#include "SampleConsensus.h"

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(Interface, *utf::disabled())

BOOST_AUTO_TEST_CASE(Sample) {
    BOOST_TEST_MESSAGE("Test: Interface: Sample");
    quintet::Interface<quintet::SampleConsensus> inf;

    inf.configure(std::string(CMAKE_SOURCE_DIR) + "/test/SampleConfig.json");

    inf.bind("duplicate", [&](std::string str, int n)->std::string {
        std::string res;
        for (int i = 0; i < n; ++i)
            res += str;
        return res;
    });

    std::ostringstream ss;
    inf.bind("print", [&ss](std::string str)->void {
        ss << str;
    });

    inf.run();

    std::string hello = "Hello, World!\n";
    auto ans = inf.call("duplicate", hello, 2);
    BOOST_REQUIRE_EQUAL(boost::any_cast<std::string>(ans), hello + hello);
}



BOOST_AUTO_TEST_SUITE_END()