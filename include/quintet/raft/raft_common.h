#pragma once

#include <string>

#include "common.h"

namespace quintet {
namespace raft {

const std::size_t IdentityNum = 3;
enum class IdentityNo { Follower = 0, Candidate, Leader, Down, Error };
const char *IdentityNames[] = {"Follower", "Candidate", "Leader", "Down",
                               "Error"};

using Term = std::uint64_t;
const Term InvalidTerm = 0;
using Index = std::size_t;

class LogEntry : public BasicLogEntry {
public:
  LogEntry(std::string opName_, std::string args_, PrmIdx prmIdx, Term term_)
      : BasicLogEntry(std::move(opName_), std::move(args_), prmIdx),
        term(term_) {}

  GEN_DEFAULT_CTOR_AND_ASSIGNMENT(LogEntry);
  GEN_CONST_HANDLE(term);

private:
  Term term = InvalidTerm;
};

} // namespace raft
} // namespace quintet