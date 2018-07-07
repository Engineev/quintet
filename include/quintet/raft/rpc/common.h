#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "../raft_common.h"

namespace quintet {
namespace raft {

class Reply {
public:
  Reply(Term term, bool success) : term(term), success(success) {}
  GEN_DEFAULT_CTOR_AND_ASSIGNMENT(Reply);
  GEN_CONST_HANDLE(term);
  GEN_CONST_HANDLE(success);

private:
  Term term = InvalidTerm;
  bool success = false;
}; // class Reply

class AppendEntriesMessage {
public:
  AppendEntriesMessage(Term term, const ServerId &leaderId, size_t prevLogIdx,
                       Term prevLogTerm, std::vector<LogEntry> logEntries,
                       size_t commitIdx)
      : term(term), leaderId(leaderId), prevLogIdx(prevLogIdx),
        prevLogTerm(prevLogTerm), logEntries(std::move(logEntries)),
        commitIdx(commitIdx) {}

  GEN_CONST_HANDLE(term);
  GEN_CONST_HANDLE(leaderId);
  GEN_CONST_HANDLE(prevLogIdx);
  GEN_CONST_HANDLE(prevLogTerm);
  GEN_CONST_HANDLE(logEntries);
  GEN_CONST_HANDLE(commitIdx);

private:
  Term term = InvalidTerm;
  ServerId leaderId;
  std::size_t prevLogIdx = 0;
  Term prevLogTerm = 0;
  std::vector<LogEntry> logEntries;
  std::size_t commitIdx = 0;
}; // class AppendEntriesMessage

class RequestVoteMessage {
public:
  RequestVoteMessage(Term term, const ServerId &candidateId, size_t lastLogIdx,
                     Term lastLogTerm)
      : term(term), candidateId(candidateId), lastLogIdx(lastLogIdx),
        lastLogTerm(lastLogTerm) {}

  GEN_CONST_HANDLE(term);
  GEN_CONST_HANDLE(candidateId);
  GEN_CONST_HANDLE(lastLogIdx);
  GEN_CONST_HANDLE(lastLogTerm);

private:
  Term term = InvalidTerm;
  ServerId candidateId;
  std::size_t lastLogIdx = 0;
  Term lastLogTerm = InvalidTerm;
}; // class RequestVoteMessage

} // namespace raft
} // namespace quintet