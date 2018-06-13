#ifndef QUINTET_RAFT_H
#define QUINTET_RAFT_H

#include <functional>
#include <string>
#include <memory>

#include "QuintetDefs.h"
#include "RaftDefs.h"

namespace quintet {

struct ServerInfo; // forward declaration

class Raft {
public:
  using ServerId = quintet::ServerId;

  Raft();

  ~Raft();

  void AddLog(std::string opName, std::string args, PrmIdx idx);

  void BindCommitter(std::function<void(LogEntry)> committer);

  ServerId Local() const;

  void Configure(const std::string & filename);

  void AsyncRun();

  void Stop();

#ifdef UNIT_TEST
  void setBeforeTransform(std::function<ServerIdentityNo(ServerIdentityNo, ServerIdentityNo)> f);

  void setAfterTransform(std::function<void(ServerIdentityNo, ServerIdentityNo)> f);

  const ServerInfo & getInfo() const;

  Term getCurrentTerm() const;

  void setRpcLatency(std::uint64_t lb, std::uint64_t ub);
#endif

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;

}; // class Raft

} // namespace quintet

#endif // QUINTET_RAFT_H
