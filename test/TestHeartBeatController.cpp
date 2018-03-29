#include <boost/test/unit_test.hpp>
#include "raft/ServerService.h"

#include <thread>

BOOST_AUTO_TEST_SUITE(Service)

BOOST_AUTO_TEST_CASE(HeartBeatController) {
    BOOST_TEST_MESSAGE("Test: HeartBeatController");
    const auto Period = 50;
    int a = 0;
    quintet::HeartBeatController hbc;
    hbc.bind([&a]{++a;}, Period);
    hbc.start();
    hbc.start();
    hbc.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(Period) * 10.2);
    hbc.stop();
    BOOST_TEST((9 <= a && a <= 11), "a = " << a);

    hbc.stop();
    hbc.stop();
    a = 0;
    hbc.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(Period) * 10.2);
    hbc.stop();
    BOOST_TEST((9 <= a && a <= 11), "a = " << a);

    a = 0;
    hbc.bind([&a]{a += 2; }, Period);
    hbc.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(Period) * 10.2);
    hbc.stop();
    BOOST_TEST((20 <= a && a <= 21), "a = " << a);

    // dtor
    auto hbc2 = new quintet::HeartBeatController([&](){
        std::this_thread::sleep_for(std::chrono::milliseconds(Period) * 5);
    }, Period);
    hbc2->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(Period));
    hbc2->stop();
    delete hbc2;
    std::this_thread::sleep_for(std::chrono::milliseconds(Period));
}

BOOST_AUTO_TEST_SUITE_END()