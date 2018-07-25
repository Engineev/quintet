#pragma once

#include <string>
#include <stdexcept>
#include <vector>

#include "macro.h"

namespace quintet {

/// The address of a server. Construct with std::string of form <ip>:<port>
class ServerId {
public:
  GEN_DEFAULT_CTOR_AND_ASSIGNMENT(ServerId);
  ServerId(std::string id_) : id(std::move(id_)) {}

  std::string toString() const { return id; }

private:
  std::string id;
}; /* class ServerId */


/// The configuration of the cluster and this server
class ServerInfo {
public:
  GEN_DEFAULT_CTOR_AND_ASSIGNMENT(ServerInfo);

  void load(const std::string & filename);

  void save(const std::string & filename);

  GEN_CONST_HANDLE(local);
  GEN_CONST_HANDLE(srvList);
  GEN_CONST_HANDLE(electionTimeout);

private:
  ServerId local;
  std::vector<ServerId> srvList;
  std::uint64_t electionTimeout = 0;
};

} /* namespace quintet */
