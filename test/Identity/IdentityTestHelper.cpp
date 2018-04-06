#include "IdentityTestHelper.h"

std::vector<std::unique_ptr<quintet::Server>> quintet::test::IdentityTestHelper::makeServers(std::size_t num) {
    assert(num <= 5);
    std::vector<std::unique_ptr<quintet::Server>> res;
    for (int i = 0; i < num; ++i) {
        auto tmp = std::make_unique<quintet::Server>();
        tmp->init(std::string(CMAKE_SOURCE_DIR) + "/test/RaftConfig/RaftConfig" + std::to_string(i) + ".json");
        res.emplace_back(std::move(tmp));
    }
    return res;
}

void quintet::test::IdentityTestHelper::runServers(std::vector<std::unique_ptr<quintet::Server>> & srvs) {
    for (auto &&srv : srvs)
        srv->run();
}
