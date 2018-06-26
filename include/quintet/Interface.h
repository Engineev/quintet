#ifndef QUINTET_INTERFACE_H
#define QUINTET_INTERFACE_H

#include <memory>
#include <unordered_map>

#include <boost/any.hpp>
#include <boost/thread/future.hpp>
#include <boost/thread/mutex.hpp>

#include "PseudoRaft.h"
#include "Raft.h"
#include "QuintetDefs.h"
#include "RaftDefs.h"
#include "Serialization.h"
#include "RaftClient.h"
#include "misc/Thread.h"

namespace quintet {

class Interface {
public:
  Interface();
  GEN_MOVE(Interface, default);
  GEN_COPY(Interface, delete);

  void Start();

  void Shutdown();

  template <class... Args>
  boost::future<boost::any> asyncCall(
      const std::string &opName, Args... rawArgs) {
    std::string args = serialize(rawArgs...);
    PrmIdx idx = pImpl->curPrmIdx++;
    boost::promise<boost::any> prm;
    auto fut = prm.get_future();
    pImpl->prms.emplace(idx, std::move(prm));
    addLog({opName, args, idx, pImpl->raft.Local()});
    return fut;
  }

  template <class... Args>
  boost::any call(const std::string &opName, Args... args) {
    return asyncCall(opName, std::move(args)...).get();
  }

  template <class Func> Interface &bind(const std::string &name, Func f) {
    bindImpl(name, std::move(f), &Func::operator());
    return *this;
  }

  void Configure(const std::string & dir) {
    pImpl->raft.Configure(dir);
  }

private:
  // Just to enable simple move operations. No actual pImpl idiom is used.
  struct Impl {
    Raft raft;
    boost::mutex cachedM;
    ServerId cachedLeader = NullServerId;
    ThreadPool threads;

    // the mapping from the names of the operation to the corresponding function
    std::unordered_map<std::string, std::function<boost::any(std::string)>> fs;
    // the promises to be set
    std::unordered_map<PrmIdx, boost::promise<boost::any>> prms;
    std::atomic<PrmIdx> curPrmIdx{0};
  };
  std::unique_ptr<Impl> pImpl;

private:
  void apply(BasicLogEntry entry);

  template <class Func, class Closure, class Ret, class... Args>
  void bindImpl(const std::string &name, Func rawF,
            Ret (Closure::*)(Args...) const) {
    pImpl->fs.insert(std::make_pair(
        name, [rawF = rawF](std::string rawArgs) -> boost::any {
          auto tup = deserialize<Args...>(rawArgs);
          auto res = boost::hana::unpack(tup, rawF);
          return res;
        }));
  }

  template <class Func, class Closure, class... Args>
  void bindImpl(const std::string &name, Func rawF,
            void (Closure::*)(Args...) const) {
    pImpl->fs.insert(std::make_pair(
        name, [rawF = rawF](std::string rawArgs) -> boost::any {
          auto tup = deserialize<Args...>(rawArgs);
          boost::hana::unpack(tup, rawF);
          return boost::any();
        }));
  }

  void addLog(BasicLogEntry entry);
};

} // namespace quintet

#endif // QUINTET_INTERFACE_H
