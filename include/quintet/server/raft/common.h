#pragma once

#include <cstdint>
#include <cstddef>
#include <utility>
#include <vector>

#include "../common.h"
#include "common/config.h"
#include "common/macro.h"

namespace quintet {

enum class IdentityNo { Follower = 0, Candidate, Leader, Down, Error };

using Term = std::uint64_t;
const Term InvalidTerm = 0;
using Index = std::size_t;

/// One log entry contains one client's command and some other information
/// the algorithm required.
class LogEntry : public BasicLogEntry {
public:
  LogEntry(std::string opName_, std::string args_, Term term_)
      : BasicLogEntry(std::move(opName_), std::move(args_)),
        term(term_) {}

  GEN_DEFAULT_CTOR_AND_ASSIGNMENT(LogEntry);
  GEN_CONST_HANDLE(term);

private:
  Term term = InvalidTerm;
}; /* class LogEntry */

/// The reply to the raft RPCs
class RpcReply {
public:
  RpcReply(Term term, bool success) : term(term), success(success) {}
  GEN_DEFAULT_CTOR_AND_ASSIGNMENT(RpcReply);
  GEN_CONST_HANDLE(term);
  GEN_CONST_HANDLE(success);

private:
  Term term = InvalidTerm;
  bool success = false;
}; // class RpcReply

/// A structure which packages the arguments of AppendEntries RPC
class AppendEntriesMessage {
public:
  AppendEntriesMessage() = default;
  AppendEntriesMessage(Term term, ServerId leaderId, size_t prevLogIdx,
                       Term prevLogTerm, std::vector<LogEntry> logEntries,
                       size_t commitIdx)
      : term(term), leaderId(std::move(leaderId)), prevLogIdx(prevLogIdx),
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

/// A structure which packages the arguments of RequestVote RPC
class RequestVoteMessage {
public:
  RequestVoteMessage(Term term, ServerId candidateId, size_t lastLogIdx,
                     Term lastLogTerm)
      : term(term), candidateId(std::move(candidateId)), lastLogIdx(lastLogIdx),
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

} /* namespace quintet */