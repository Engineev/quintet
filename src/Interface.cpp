#include "Interface.h"

#include <boost/thread/locks.hpp>

#include "service/rpc/RpcClient.h"
#include "ServerInfo.h"
#include "service/log/Common.h"

namespace quintet {

Interface::Interface() : pImpl(std::make_unique<Impl>()) {}

void Interface::Start() {
  pImpl->raft.BindApply(
      std::bind(&Interface::apply, this, std::placeholders::_1));
  pImpl->raft.AsyncRun();
}

void Interface::Shutdown() {
  pImpl->raft.Stop();
  pImpl->threads.clearWithInterruption();
}

void Interface::apply(BasicLogEntry entry) {
  auto res = pImpl->fs.at(entry.get_opName())(entry.get_args());
  if (entry.get_srvId() == pImpl->raft.Local()) {
    BOOST_LOG(pImpl->raft.getLogger()) << "prmIdx(out) = " << entry.get_prmIdx();
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

    const auto srvList = pImpl->raft.getInfo().srvList;
    auto iter = srvList.cbegin(), end = srvList.cend();
    while (true) {
      auto & srv = *iter;
      ++iter;

//      BOOST_LOG(pImpl->raft.getLogger())
//        << "trying " << srv.toString();

      RaftClient client(srv);
      AddLogReply reply;
      try {
        reply = client.callRpcAddLog(rpc::makeClientContext(50), msg);
      } catch (rpc::RpcError & e) {
        reply.success = false;
      }
      if (reply.success) {
        BOOST_LOG(pImpl->raft.getLogger())
          << "Succeeded. Leader = " << srv.toString();
        lk.lock();
        pImpl->cachedLeader = srv;
        return;
      }
      std::this_thread::yield();
      if (iter == end)
        iter = srvList.begin();
    }
  }));
}

} // namespace quintet