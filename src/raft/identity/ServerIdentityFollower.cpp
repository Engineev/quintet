#include "ServerIdentityFollower.h"

#include <ctime>
#include <random>

namespace quintet {
ServerIdentityFollower::ServerIdentityFollower(ServerState &state_,
                                               ServerInfo &info_,
                                               ServerService &service_)
    : ServerIdentityBase(state_, info_, service_) {}

void ServerIdentityFollower::init() {
    ++state.currentTerm;

    // launch Leader Checker
    std::random_device rd;
    std::default_random_engine eg(rd());
    electionTimeout =
        info.electionTimeout + std::uniform_int_distribution<std::uint64_t>(
                                   0, info.electionTimeout)(eg);

    service.logger("\n\tFollower::init()\n\telectionTimeout = ",
                   electionTimeout);

    service.heartBeatController.oneShot(
        [&] {
            service.identityTransformer.transform(ServerIdentityNo::Candidate);
        },
        electionTimeout);
}

void ServerIdentityFollower::leave() {}

std::pair<Term /*current term*/, bool /*success*/>
ServerIdentityFollower::RPCAppendEntries(Term term, ServerId leaderId,
                                         std::size_t prevLogIdx,
                                         Term prevLogTerm,
                                         std::vector<LogEntry> logEntries,
                                         std::size_t commitIdx) {
    // TODO
    updateCurrentTerm(term);
    boost::shared_lock<boost::shared_mutex> readCurrentTermLk(currentTermM);

    if (term < state.currentTerm) {
        return {state.currentTerm, false};
    }

    resetHeartBeat();

    boost::upgrade_lock<boost::shared_mutex> readEntriesLk(entriesM);
    if (state.entries.size() <= prevLogIdx ||
        state.entries.at(prevLogIdx).term != prevLogTerm) {
        return {state.currentTerm, false};
    }

    for (Index idxOffset = 0; idxOffset < logEntries.size(); ++idxOffset) {
        Index entriesIdx = idxOffset + prevLogIdx;
        if (state.entries.at(entriesIdx).term !=
            logEntries.at(idxOffset).term) {
            boost::upgrade_to_unique_lock<boost::shared_mutex> writeEntriesLk(
                readEntriesLk);
            state.entries.erase(state.entries.begin() + entriesIdx,
                                state.entries.end());
            state.entries.insert(state.entries.end(),
                                 logEntries.begin() + idxOffset,
                                 logEntries.end());
            break;
        }
    }

    if (commitIdx > state.commitIdx) {
        if (state.entries.size() == 0) {
            throw std::runtime_error("rescieve commit when own log is empty");
        }
        Index newCommitIdx = std::min(commitIdx, state.entries.size() - 1);
        for (Index commitItem = state.commitIdx + 1; commitItem <= newCommitIdx;
             ++commitItem) {
            service.committer.commit(state.entries.at(commitItem));
        }
        state.commitIdx = newCommitIdx;
    }

    return {state.currentTerm, true};
}

std::pair<Term /*current term*/, bool /*vote granted*/>
ServerIdentityFollower::RPCRequestVote(Term term, ServerId candidateId,
                                       std::size_t lastLogIdx,
                                       Term lastLogTerm) {
    updateCurrentTerm(term);
    boost::shared_lock<boost::shared_mutex> readCurrentTermLk(currentTermM);
    if (term < state.currentTerm) return {state.currentTerm, false};
    boost::shared_lock<boost::shared_mutex> readEntriesLk(entriesM);
    if ((state.votedFor == NullServerId || state.votedFor == candidateId) &&
        upToDate(state, lastLogIdx, lastLogTerm)) {
        state.votedFor = candidateId;
        return {state.currentTerm, true};
    }
    return {state.currentTerm, false};
}

// Private
void ServerIdentityFollower::resetHeartBeat() {
    // Restart heart beat time
    service.heartBeatController.resetOneShots();
    service.heartBeatController.oneShot(
        [&] {
            service.identityTransformer.transform(ServerIdentityNo::Candidate);
        },
        electionTimeout);
}

void ServerIdentityFollower::updateCurrentTerm(Term term) {
    boost::unique_lock<boost::shared_mutex> writeCurrentTermLk(currentTermM);
    if (term > state.currentTerm) {
        state.currentTerm = term;
    }
}
} // namespace quintet