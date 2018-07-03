#ifndef QUINTET_ERROR_H
#define QUINTET_ERROR_H

#include <string>
#include <exception>

#include "common.h"

namespace quintet {
namespace rpc {

class Error : public std::exception {
public:
  Error() = default;
  explicit Error(std::string m) : msg(std::move(m)) {}

  const char * what() const noexcept override { return msg.c_str(); }

private:
  std::string msg;
};

class NotLeader : public Error {
public:
  NotLeader() = default;
  explicit NotLeader(ServerId leader)
      : Error("The server queried is not the leader. Current leader is "
              + leader.toString()),
        leaderId(std::move(leader)) {}

  GEN_CONST_HANDLE(leaderId);

private:
  ServerId leaderId;

};

} // namespace rpc
} // namespace quintet

#endif //QUINTET_ERROR_H
