#ifndef QUINTET_COMMON_H
#define QUINTET_COMMON_H

#include "common/config.h"
#include "common/macro.h"

namespace quintet {

using DebugId = int;

/// A wrapper of the client's command
class BasicLogEntry {
public:
  BasicLogEntry(std::string opName_, std::string args_)
      : opName(std::move(opName_)), args(std::move(args_)){}

  GEN_DEFAULT_CTOR_AND_ASSIGNMENT(BasicLogEntry);
  GEN_CONST_HANDLE(opName);
  GEN_CONST_HANDLE(args);

private:
  std::string opName;
  std::string args;
}; // class BasicLogEntry

} /* namespace quintet */

#endif //QUINTET_COMMON_H
