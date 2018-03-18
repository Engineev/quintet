#include <boost/test/unit_test.hpp>
#include "quintet/Interface.h"

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

BOOST_AUTO_TEST_SUITE(Interface)

BOOST_AUTO_TEST_CASE(Compile) {
    BOOST_TEST_MESSAGE("Test: Interface: Compile");
    quintet::Interface<int> inf; // TODO: Sample

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
}



BOOST_AUTO_TEST_SUITE_END()