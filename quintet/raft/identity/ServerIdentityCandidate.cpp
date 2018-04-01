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
        std::vector<quintet::FutureWrapper<clmdep_msgpack::object_handle>> &&votes) {
    voteResults.resize(votes.size());
    for (auto &&voteResult : voteResults)
        voteResult = std::make_shared<VoteResult>(VoteResult::Unready);

    for (int i = 0; i < (int)votes.size(); ++i) {
        votes[i].then([this, voteResult = voteResults[i]]
                              (boost::future<RPCLIB_MSGPACK::object_handle> fut) mutable {
            Term termReceived;
            bool res;
            std::tie(termReceived, res) = fut.get().as<std::pair<Term, bool>>();

            // TODO: when larger term is received ...

            boost::lock_guard<boost::mutex> lk(*m);
            assert((*voteResult == VoteResult::Discarded || *voteResult == VoteResult::Unready));
            if (*voteResult == VoteResult::Discarded)
                return;
            *voteResult = res ? VoteResult::Accepted : VoteResult::Rejected;
            votesReceived += res;
            if (votesReceived > info.srvList.size())
                service.identityTransformer.transform(ServerIdentityNo::Leader);
        });
    }
}
