#include "Interface.h"

#include <boost/thread/locks.hpp>

#include "service/rpc/RpcClient.h"
#include "ServerInfo.h"

namespace quintet {

Interface::Interface() : pImpl(std::make_unique<Impl>()) {}

void Interface::Start() {
  pImpl->raft.BindApply(
      std::bind(&Interface::apply, this, std::placeholders::_1));
  pImpl->raft.AsyncRun();
}

void Interface::Shutdown() { pImpl->raft.Stop(); }

void Interface::apply(BasicLogEntry entry) {
  auto res = pImpl->fs.at(entry.get_opName())(entry.get_args());
  if (entry.get_srvId() == pImpl->raft.Local()) {
    auto prm = pImpl->prms.find(entry.get_prmIdx());
    assert(prm != pImpl->prms.end());
    prm->second.set_value(res);
    pImpl->prms.erase(prm);
  }
}

void Interface::addLog(BasicLogEntry entry) {
  pImpl->threads.add(boost::thread([entry, this] {
    AddLogMessage msg;
    msg.srvId = pImpl->raft.Local();
    msg.opName = entry.get_opName();
    msg.args = entry.get_args();
    msg.prmIdx = entry.get_prmIdx();
    boost::unique_lock<boost::mutex> lk(pImpl->cachedM);
    if (pImpl->cachedLeader != NullServerId) {
      RaftClient client(pImpl->cachedLeader);
      auto reply = client.callRpcAddLog(rpc::makeClientContext(50), msg);
      if (reply.success)
        return;
    }
    lk.unlock();

    for (auto & srv : pImpl->raft.getInfo().srvList) { // TODO
      RaftClient client(srv);
      auto reply = client.callRpcAddLog(rpc::makeClientContext(50), msg);
      if (reply.success) {
        lk.lock();
        pImpl->cachedLeader = srv;
        return;
      }
    }
  }));
}

} // namespace quintet