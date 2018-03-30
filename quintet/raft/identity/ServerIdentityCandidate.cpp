#include "ServerIdentityCandidate.h"

quintet::ServerIdentityCandidate::ServerIdentityCandidate(quintet::ServerState &state_, quintet::ServerInfo &info_,
                                                          quintet::ServerService &service_)
        : ServerIdentityBase(state_, info_, service_) {}

std::vector<quintet::FutureWrapper<clmdep_msgpack::object_handle>> quintet::ServerIdentityCandidate::sendRequests() {
    std::vector<FutureWrapper<RPCLIB_MSGPACK::object_handle>> res;
    for (auto && srv : info.srvList) {
        res.emplace_back(service.rpcService.async_call(
                srv.addr, srv.port, "RequestVote",
                state.currentTerm, info.local,
                state.entries.size() - 1, state.entries.back().term
        ));
    }
    return res;
}
