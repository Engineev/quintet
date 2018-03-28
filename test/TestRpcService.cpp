#include <boost/test/unit_test.hpp>
#include "raft/ServerService.h"

#include <vector>
#include <memory>
#include <thread>
#include <chrono>
#include <random>
#include <iostream>
#include <utility>
#include <functional>

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(Service)

struct RPCs {
    std::vector<std::unique_ptr<quintet::RpcService>> rpcs;
    const int ServerNum;
    const quintet::Port BasePort;

    RPCs() : ServerNum(5), BasePort(2000) {
        for (int i = 0; i < ServerNum; ++i)
            rpcs.emplace_back(std::make_unique<quintet::RpcService>());

        quintet::Port port = BasePort;
        for (auto &&rpc : rpcs) {
            rpc->listen(port++);
            rpc->bind("add", [](int a, int b) { return a + b; })
                    .bind("sleepFor", [](int ms) -> void {
                        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
                    });
            rpc->async_run();
        }
    }
};

BOOST_FIXTURE_TEST_SUITE(RpcService, RPCs, *utf::disabled())

BOOST_AUTO_TEST_CASE(RpcService_Advanced) {
    BOOST_TEST_MESSAGE("Test: RpcService_Advanced");
    using namespace std::chrono_literals;
    std::vector<std::thread> ts;
    const auto TestDuration = 3s;

    auto foo = [&](int srv) {
        std::vector<std::pair<quintet::FutureWrapper<RPCLIB_MSGPACK::object_handle>, int>> futs;
        // int ans; ans < 0 means it is created by "sleepFor"
        auto & rpc = *rpcs[srv];

        std::default_random_engine eng;
        std::uniform_int_distribution<int> distTime(0, 10); // ms
        auto randTime = std::bind(distTime, eng);
        auto randInt  = randTime;
        std::uniform_int_distribution<int> distSrv(0, ServerNum - 1);
        auto randPort = [&] {
            return quintet::Port(distSrv(eng) + BasePort);
        };

        std::vector<std::function<void()>> op = {
                [&] { // sleep
                    std::this_thread::sleep_for(
                            std::chrono::milliseconds(randTime()));
                },
                [&] /* pause  */ { rpc.pause(); },
                [&] /* resume */ { rpc.resume(); },
                [&] { // add
                    int x = randInt(), y = randInt();
                    int ans = x + y;
                    if (randInt() % 2)
                        BOOST_REQUIRE_EQUAL(ans,
                                            rpc.call("localhost", randPort(), "add", x, y).as<int>());
                    else
                        futs.emplace_back(rpc.async_call("localhost", randPort(), "add", x, y), ans);
                },
                [&] { // sleepFor
                    if (randInt() % 2)
                        rpc.call("localhost", randPort(), "sleepFor", randTime());
                    else
                        futs.emplace_back(rpc.async_call("localhost", randPort(), "sleepFor", randTime()), -1);
                }
        };
        std::uniform_int_distribution<std::size_t> distOp(0, op.size() - 1);
        auto randOp = std::bind(distOp, eng);


        const auto Start = std::chrono::high_resolution_clock::now();
        while (std::chrono::high_resolution_clock::now() - Start < TestDuration) {
            op[randOp()]();
        }

        // test
        for (auto && item : futs) {
            if (item.second != -1)
                BOOST_REQUIRE_EQUAL(item.second, item.first.get().as<int>());
        }
    };


    for (int i = 0; i < ServerNum; ++i)
        ts.emplace_back(std::thread(foo, i));


    std::this_thread::sleep_for(TestDuration + 1s);

    // clean up
    for (auto && rpc : rpcs)
        rpc->resume();
    for (auto && t : ts)
        t.join();
    BOOST_REQUIRE(true); // TODO: RpcService_Advanced: check
} // case RpcService_Advanced

BOOST_AUTO_TEST_CASE(RpcService_Naive) {
    BOOST_TEST_MESSAGE("Test: RpcService_Naive");

    using namespace std::chrono_literals;

    BOOST_REQUIRE_EQUAL(2, rpcs[0]->call("localhost", BasePort + 2, "add", 1, 1).as<int>());
    BOOST_REQUIRE_NO_THROW(rpcs[1]->async_call("localhost", BasePort, "sleepFor", 10));
    rpcs[0]->pause();
    auto fut = rpcs[1]->async_call("localhost", BasePort, "add", 1, 1);
    std::this_thread::sleep_for(10ms);
    BOOST_REQUIRE_EQUAL(false, fut.ready());
    rpcs[0]->resume();
    std::this_thread::sleep_for(10ms);
    BOOST_REQUIRE_EQUAL(true, fut.ready());
} // case RpcService_Naive

BOOST_AUTO_TEST_SUITE_END() // suite RpcService

BOOST_AUTO_TEST_SUITE_END()