#include "raft/raft.h"

#include <array>

#include <boost/thread/future.hpp>

#include "raft/identity/identity.h"
#include "misc/event_queue.h"

namespace quintet {
namespace raft {

struct Raft::Impl {
  std::array<std::unique_ptr<IdentityBase>, IdentityNum> identities;
  IdentityNo curIdentityNo = IdentityNo::Down;

  EventQueue eventQueue; // to synchronize between transformations and 'AddLog's



}; // struct Raft::Impl

} // namespace raft
} // namespace quintet

namespace quintet {
namespace raft {

std::pair<bool, ServerId> Raft::AddLog(BasicLogEntry entry) {
  boost::promise<std::pair<bool, ServerId>> prm;
  auto fut = prm.get_future();
  pImpl->eventQueue.addEvent([this, &prm] {
    throw ; // TODO
  });
  return fut.get();
}

} // namespace raft
} // namespace quintet