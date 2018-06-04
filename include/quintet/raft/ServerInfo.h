#pragma once

#include <cstdint>
#include <vector>

#include "RaftDefs.h"

// ServerInfo
namespace quintet {

struct ServerInfo {
  ServerId local;
  std::vector<ServerId> srvList;
  std::uint64_t electionTimeout;
  // TODO: timeout

  void load(const std::string & filename);

  void save(const std::string & filename);
};

} // namespace quintet