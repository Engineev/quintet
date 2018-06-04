#ifndef QUINTET_RAFT_H
#define QUINTET_RAFT_H

#include <functional>
#include <string>

#include "QuintetDefs.h"
#include "RaftDefs.h"

namespace quintet {

class Raft {
public:
  using ServerId = ServerId;

  Raft();

  ~Raft();

  void AddLog(std::string opName, std::string args, PrmIdx idx);

  void BindCommitter(
      std::function<void(std::string, std::string, ServerId, quintet::PrmIdx)>
          committer);

  ServerId Local() const;

  void Configure(const std::string & filename);

  void run();

  void stop();

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;

}; // class Raft

} // namespace quintet

#endif // QUINTET_RAFT_H
