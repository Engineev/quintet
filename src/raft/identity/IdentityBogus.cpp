#include "identity/IdentityBogus.h"

namespace quintet {

std::pair<Term, bool>
IdentityBogus::RPCAppendEntries(AppendEntriesMessage message) {}

std::pair<Term, bool>
IdentityBogus::RPCRequestVote(RequestVoteMessage message) {}

void IdentityBogus::init() {}

void IdentityBogus::leave() {}



} // namespace quintet