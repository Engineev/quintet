#include "ServerState.h"

bool quintet::upToDate(const quintet::ServerState &state, std::size_t lastLogIdx, quintet::Term lastLogTerm) {
    if (state.entries.empty()) return true;
    if (state.entries.back().term < lastLogTerm) return true;

    // candidate is up to date when
    // the two log lengths are equal
    if (state.entries.back().term == lastLogTerm &&
        state.entries.size() - 1 <= lastLogIdx)
        return true;

    return false;
}
