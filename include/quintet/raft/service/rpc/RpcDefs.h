#ifndef QUINTET_RPCDEFS_H
#define QUINTET_RPCDEFS_H

#include <cstdint>
#include <vector>

#include "RaftDefs.h"

namespace quintet {

using Reply = std::pair<Term, bool>;

struct AppendEntriesMessage {
  Term term;
  ServerId leaderId;
  std::size_t prevLogIdx;
  Term prevLogTerm;
  std::vector<LogEntry> logEntries;
  std::size_t commitIdx;

  AppendEntriesMessage() = default;
  AppendEntriesMessage(const AppendEntriesMessage &) = default;
  AppendEntriesMessage(AppendEntriesMessage &&) = default;
  AppendEntriesMessage& operator=(const AppendEntriesMessage &) = default;
  AppendEntriesMessage& operator=(AppendEntriesMessage &&) = default;
};

struct RequestVoteMessage {
  Term term;
  ServerId candidateId;
  std::size_t lastLogIdx;
  Term lastLogTerm;

  RequestVoteMessage() = default;
  RequestVoteMessage(const RequestVoteMessage &) = default;
  RequestVoteMessage(RequestVoteMessage &&) = default;
  RequestVoteMessage& operator=(const RequestVoteMessage &) = default;
  RequestVoteMessage& operator=(RequestVoteMessage &&) = default;
};

}

#endif //QUINTET_RPCDEFS_H
