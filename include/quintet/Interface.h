#ifndef QUINTET_INTERFACE_H
#define QUINTET_INTERFACE_H

#include <memory>
#include <unordered_map>

#include <boost/any.hpp>
#include <boost/thread/future.hpp>

#include "QuintetDefs.h"
#include "RaftDefs.h"
#include <Raft.h>

namespace quintet {

class Interface {
public:
  Interface();
  Interface(const Interface &) = delete;
  Interface(Interface &&) = default;
  Interface &operator=(const Interface &) = delete;
  Interface &operator=(Interface &&) = default;

  template <class... Args>
  boost::future<boost::any> asyncCall(const std::string &opName,
                                      Args... rawArgs);

  template <class... Args>
  boost::any call(const std::string &opName, Args... args);

  template <class Func> Interface &bind(const std::string &name, Func f);

private:
  // Just to enable simple move operations. No actual pImpl idiom is used.
  struct Impl {
    // the mapping from the names of the operation to the corresponding function
    std::unordered_map<std::string, std::function<boost::any(std::string)>> fs;
    // the promises to be set
    std::unordered_map<PrmIdx, boost::promise<boost::any>> prms;
    std::atomic<PrmIdx> curPrmIdx{0};
    Raft raft;

    void addLog(const std::string & name, const std::string & args, PrmIdx prmIdx);
  };
  std::unique_ptr<Impl> pImpl;
};

} // namespace quintet

#endif // QUINTET_INTERFACE_H
