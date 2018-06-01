#include "ServerIdentityLeader.h"

#include <unordered_map>

/* -------------- constructors, destructors and Impl ------------------------ */

namespace quintet {

ServerIdentityLeader::~ServerIdentityLeader() = default;

struct ServerIdentityLeader::Impl : public IdentityBaseImpl {
    Impl(ServerState & state, ServerInfo & info, ServerService & service)
        : IdentityBaseImpl(state, info, service) {}

    std::unordered_map<ServerId, Index> nextIndex, matchIndex;

    void reinit();


};

ServerIdentityLeader::ServerIdentityLeader(
    ServerState &state, ServerInfo &info, ServerService &service)
    : pImpl(std::make_unique<Impl>(state, info, service)) {}

} // namespace quintet


/* ---------------- public member functions --------------------------------- */

namespace quintet {

void ServerIdentityLeader::init() {
    
}

} // namespace quintet

/* ---------------------- RPCs ---------------------------------------------- */

namespace quintet {} // namespace quintet

/* -------------------- Impl functions -------------------------------------- */

namespace quintet {} // namespace quintet