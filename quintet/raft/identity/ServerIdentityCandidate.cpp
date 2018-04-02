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

void quintet::ServerIdentityCandidate::launchVotesChecker(
        std::vector<quintet::FutureWrapper<RPCLIB_MSGPACK::object_handle>> &&votes) {
    for (auto && vote : votes) {
        // capture data by value !!
        vote.then([this, data = this->data](boost::future<RPCLIB_MSGPACK::object_handle> fut) mutable {
            Term termReceived;
            bool res;
            std::tie(termReceived, res) = fut.get().as<std::pair<Term, bool>>();

            // TODO: when larger term is received ...

            boost::lock_guard<boost::mutex> lk(data->m);
            if (data->discarded)
                return;
            data->votesReceived += res;
            // It is guaranteed that only one transformation will be carried out.
            if (data->votesReceived > info.srvList.size())
                service.identityTransformer.transform(ServerIdentityNo::Leader);
        });
    }
}
