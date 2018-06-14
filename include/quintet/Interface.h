#ifndef QUINTET_INTERFACE_H
#define QUINTET_INTERFACE_H

#include <unordered_map>

#include <boost/any.hpp>
#include <boost/thread/future.hpp>

#include "QuintetDefs.h"
#include "RaftDefs.h"

namespace quintet {

class Interface {
public:
  template <class... Args>
  boost::future<boost::any> asyncCall(const std::string & opName, Args... rawArgs);

  template <class... Args>
  boost::any call(const std::string & opName, Args... args);

  template <class Func>
  Interface & bind(const std::string & name, Func f);

private:
  // the mapping from the names of the operation to the corresponding function
  std::unordered_map<std::string, std::function<boost::any(std::string)>> fs;
  // the promises to be set
  std::unordered_map<PrmIdx, boost::promise<boost::any>> prms;
  std::atomic<PrmIdx> prmIdx{0};
};

} // namespace quintet

#endif //QUINTET_INTERFACE_H
