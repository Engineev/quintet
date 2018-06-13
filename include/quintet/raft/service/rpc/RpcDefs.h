#ifndef QUINTET_RPCDEFS_H
#define QUINTET_RPCDEFS_H

#include <cstdint>
#include <vector>
#include <stdexcept>

#include "RaftDefs.h"

namespace quintet {
namespace rpc {
class RpcError : public std::exception {
public:
  RpcError() = default;
  explicit RpcError(std::string m) : msg(std::move(m)) {}

  const char * what() const noexcept override { return msg.c_str(); }

private:
  std::string msg;
};
}
}

namespace quintet {

using Reply = std::pair<Term, bool>;

struct AppendEntriesMessage {
  Term term = InvalidTerm;
  ServerId leaderId;
  std::size_t prevLogIdx = 0;
  Term prevLogTerm = 0;
  std::vector<LogEntry> logEntries;
  std::size_t commitIdx = 0;

  AppendEntriesMessage() = default;
  AppendEntriesMessage(const AppendEntriesMessage &) = default;
  AppendEntriesMessage(AppendEntriesMessage &&) = default;
  AppendEntriesMessage& operator=(const AppendEntriesMessage &) = default;
  AppendEntriesMessage& operator=(AppendEntriesMessage &&) = default;
};

struct RequestVoteMessage {
  Term term = InvalidTerm;
  ServerId candidateId;
  std::size_t lastLogIdx = 0;
  Term lastLogTerm = InvalidTerm;

  RequestVoteMessage() = default;
  RequestVoteMessage(const RequestVoteMessage &) = default;
  RequestVoteMessage(RequestVoteMessage &&) = default;
  RequestVoteMessage& operator=(const RequestVoteMessage &) = default;
  RequestVoteMessage& operator=(RequestVoteMessage &&) = default;
};

}

#endif //QUINTET_RPCDEFS_H
