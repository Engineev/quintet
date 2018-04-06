#ifndef QUINTET_IDENTITYTESTHELPER_H
#define QUINTET_IDENTITYTESTHELPER_H

#include <vector>
#include <memory>

#include "Server.h"


namespace quintet {
namespace test {

struct IdentityTestHelper {
    std::vector<std::unique_ptr<quintet::Server>> makeServers(std::size_t num = 5);

    void runServers(std::vector<std::unique_ptr<quintet::Server>> & srvs);
};

} // namespace test
} // namespace quintet


#endif //QUINTET_IDENTITYTESTHELPER_H
