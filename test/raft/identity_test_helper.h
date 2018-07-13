#ifndef QUINTET_IDENTITY_TEST_HELPER_H
#define QUINTET_IDENTITY_TEST_HELPER_H

#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "raft/raft.h"
#include "raft/rpc/rpc_client.h"
#include "server_info.h"
#include "raft/raft_common.h"

namespace quintet {
namespace test {

class IdentityTestHelper {
public:
  std::vector<std::unique_ptr<raft::Raft>> makeServers(std::size_t num = 3) {
    assert(num <= 3);
    std::vector<std::unique_ptr<raft::Raft>> res;
    info.resize(num);
    for (int i = 0; i < (int) num; ++i) {
      info[i].load(std::string(CMAKE_SOURCE_DIR) +
          "/test/raft/config/raft_" + std::to_string(i) + ".json");
      res.emplace_back(std::make_unique<raft::Raft>(info[i]));
    }
    return res;
  }

  void sendHeartBeat(const std::vector<ServerId> &srvs, const ServerId &local,
                     quintet::raft::Term currentTerm) {
    for (auto &srv : srvs) {
      if (srv == local)
        continue;
      quintet::raft::rpc::Client client(srv);
      ClientContext ctx;
      ctx.setTimeout(100);
      quintet::raft::AppendEntriesMessage msg(currentTerm, {}, 0, 0, {}, 0);
      client.callRpcAppendEntries(ctx, msg);
    }
  }

private:
  std::vector<ServerInfo> info;

}; // class IdentityTestHelper

} // namespace test
} // namespace quintet

#endif //QUINTET_IDENTITY_TEST_HELPER_H
