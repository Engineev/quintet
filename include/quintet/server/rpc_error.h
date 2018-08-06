#include <utility>

#ifndef QUINTET_RPC_ERROR_H
#define QUINTET_RPC_ERROR_H

#include <exception>
#include <string>

#include "common/macro.h"

namespace quintet {

class RpcError : public std::exception {
public:
  enum class Code { UNKNOWN, TLE };

  GEN_DEFAULT_CTOR_AND_ASSIGNMENT(RpcError)
  explicit RpcError(Code code) : code(code) {}
  RpcError(Code code, std::string str) : code(code), msg(std::move(str)) {}

  const Code errorCode() const { return code; }

  const char *what() const noexcept override {
    return msg.c_str();
  }

private:
  Code code = Code::UNKNOWN;
  std::string msg;
};

} // namespace quintet

#endif //QUINTET_RPC_ERROR_H
