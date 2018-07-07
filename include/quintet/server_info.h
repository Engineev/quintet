#pragma once

#include <cstdint>
#include <vector>

#include "common.h"

namespace quintet {

class ServerInfo {
public:
  void load(const std::string & filename);

  void save(const std::string & filename);

  GEN_CONST_HANDLE(local);
  GEN_CONST_HANDLE(srvList);
  GEN_CONST_HANDLE(electionTimeout);

private:
  ServerId local;
  std::vector<ServerId> srvList;
  std::uint64_t electionTimeout;
};

} // namespace quintet