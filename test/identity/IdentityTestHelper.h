#ifndef QUINTET_IDENTITYTESTHELPER_H
#define QUINTET_IDENTITYTESTHELPER_H

#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Raft.h"

namespace quintet {
namespace test {

class IdentityTestHelper {
public:
  using No = ServerIdentityNo;

  std::vector<std::unique_ptr<Raft>> makeServers(std::size_t num = 3);

  void runServers(std::vector<std::unique_ptr<Raft>> &srvs);

  void sendHeartBeat(const std::vector<ServerId> &srvs, const ServerId &local,
                     Term currentTerm);

};

} // namespace test
} // namespace quintet

#endif // QUINTET_IDENTITYTESTHELPER_H
