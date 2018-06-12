#include "IdentityTestHelper.h"

#ifdef false

namespace quintet {
namespace test {

std::vector<std::unique_ptr<Raft>>
IdentityTestHelper::makeServers(std::size_t num) {
  assert(num <= 5);
  std::vector<std::unique_ptr<Raft>> res;
  for (int i = 0; i < (int)num; ++i) {
    auto tmp = std::make_unique<Raft>();
    tmp->init(std::string(CMAKE_SOURCE_DIR) + "/test/RaftConfig/RaftConfig" +
              std::to_string(i) + ".json");
    res.emplace_back(std::move(tmp));
  }

  clients.clear();
  for (auto &srv : res) {
    auto id = srv->getInfo().local;
    auto c = std::make_unique<rpc::client>(id.addr, id.port);
    c->set_timeout(5000);
    clients.emplace(id.toString(), std::move(c));
  }

  return res;
}

void IdentityTestHelper::runServers(
    std::vector<std::unique_ptr<Server>> &srvs) {
  for (auto &&srv : srvs)
    srv->run();
}

void IdentityTestHelper::sendHeartBeat(const std::vector<ServerId> &srvs,
                                       const ServerId &local,
                                       Term currentTerm) {
  for (auto &srv : srvs) {
    if (srv == local)
      continue;
    try {
      clients.at(srv.toString())
          ->call("AppendEntries", currentTerm, local, 0, 0,
                 std::vector<LogEntry>(), 0);
    } catch (rpc::timeout &e) {
    }
  }
}

} // namespace test
} // namespace quintet

#endif