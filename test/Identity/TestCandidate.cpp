#include <boost/test/unit_test.hpp>
#include "Server.h"

#include <memory>
#include <vector>
#include <string>

BOOST_AUTO_TEST_SUITE(Identity)

struct ServerFixture {
    std::vector<std::unique_ptr<quintet::Server>> makeServers() {
        std::vector<std::unique_ptr<quintet::Server>> res;
        for (int i = 0; i < 5; ++i) {
            auto tmp = std::make_unique<quintet::Server>();
            tmp->init(std::string(CMAKE_SOURCE_DIR) + "/test/RaftConfig" + std::to_string(i) + ".json");
            res.emplace_back(std::move(tmp));
        }
        return res;
    };
};

BOOST_FIXTURE_TEST_SUITE(Candidate, ServerFixture)

BOOST_AUTO_TEST_CASE(Election) {
    auto srvs = makeServers();
    for (auto && srv : srvs)
        srv->run();
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()