#ifndef QUINTET_SERIALIZATION_H
#define QUINTET_SERIALIZATION_H

#include <sstream>
#include <stdexcept>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>

#include "common/macro.h"

namespace quintet {

template <class T>
std::string serialize(const T & x) {
  std::stringstream ss;
  {
    boost::archive::text_oarchive oa(ss);
    oa << x;
  }
  return std::string{std::istreambuf_iterator<char>(ss), {}};
}

template <class T>
T deserialize(const std::string & str) {
  T res;
  std::stringstream ss(str);
  boost::archive::text_iarchive ia(ss);
  ia >> res;
  return res;
}

class Object {
public:
  GEN_DEFAULT_CTOR_AND_ASSIGNMENT(Object);
  template <class T>
  Object(const T & x) : buffer(serialize(x)) {}

  const std::string & toString() const {
    return buffer;
  }

  template <class T>
  T as() const {
    return deserialize<T>(buffer);
  }

private:
  std::string buffer;

}; // class Object

} // namespace quintet

#endif //QUINTET_SERIALIZATION_H
