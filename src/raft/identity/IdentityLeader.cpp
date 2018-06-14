#include "identity/IdentityLeader.h"

#include <boost/thread/locks.hpp>

#include "misc/Rand.h"
#include "service/log/Common.h"

/* -------------- constructors, destructors and Impl ------------------------ */

namespace quintet {

struct IdentityLeader::Impl : public IdentityBaseImpl {
  Impl(ServerState &state, ServerInfo &info, ServerService &service)
      : IdentityBaseImpl(state, info, service) {
    service.logger.add_attribute(
        "Part", logging::attrs::constant<std::string>("Identity"));
  }

  void init();

  void leave();

  Reply appendEntries(const AppendEntriesMessage &msg);

  Reply requestVote(const RequestVoteMessage &msg);

  void sendHeartBeats();

  Reply sendAppendEntries(std::vector<LogEntry> entries);

}; // struct IdentityLeader::Impl

IdentityLeader::IdentityLeader(ServerState &state, ServerInfo &info,
                                   ServerService &service)
    : pImpl(std::make_unique<Impl>(state, info, service)) {}

IdentityLeader::~IdentityLeader() = default;

} // namespace quintet

/* --------------- public member functions & RPC ---------------------------- */

namespace quintet {

void IdentityLeader::init() { pImpl->init(); }

void IdentityLeader::leave() { pImpl->leave(); }

Reply IdentityLeader::RPCAppendEntries(AppendEntriesMessage message) {
  return pImpl->appendEntries(message);
}

std::pair<Term /*current term*/, bool /*vote granted*/>
IdentityLeader::RPCRequestVote(RequestVoteMessage message) {
  return pImpl->requestVote(message);
}

} // namespace quintet

/* -------------------- Helper functions ------------------------------------ */

namespace quintet {

void IdentityLeader::Impl::init() {

}

void IdentityLeader::Impl::leave() {

}

Reply IdentityLeader::Impl::appendEntries(const AppendEntriesMessage &msg) {

}

Reply IdentityLeader::Impl::requestVote(const RequestVoteMessage &msg) {

}

} // namespace quintet