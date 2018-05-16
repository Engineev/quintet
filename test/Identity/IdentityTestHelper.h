#ifndef QUINTET_IDENTITYTESTHELPER_H
#define QUINTET_IDENTITYTESTHELPER_H

#include <cassert>
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

#include <rpc/client.h>

#include "Server.h"


namespace quintet {
namespace test {

class IdentityTestHelper {
public:
    using No = ServerIdentityNo;

    std::vector<std::unique_ptr<Server>> makeServers(std::size_t num = 3);

    void runServers(std::vector<std::unique_ptr<Server>> & srvs);

    void sendHeartBeat(const std::vector<ServerId> & srvs,
                       const ServerId & local, Term currentTerm);

private:
    std::unordered_map<std::string, std::unique_ptr<rpc::client>> clients;
};

} // namespace test
} // namespace quintet


#endif //QUINTET_IDENTITYTESTHELPER_H
