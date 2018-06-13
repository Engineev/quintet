#include "identity/IdentityBogus.h"

namespace quintet {

std::pair<Term, bool>
IdentityBogus::RPCAppendEntries(AppendEntriesMessage message) {
  return {0, 0};
}

std::pair<Term, bool>
IdentityBogus::RPCRequestVote(RequestVoteMessage message) {
  return {0, 0};
}

void IdentityBogus::init() {}

void IdentityBogus::leave() {}



} // namespace quintet