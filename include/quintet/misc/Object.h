#ifndef QUINTET_OBJECT_H
#define QUINTET_OBJECT_H

#include <string>

#include <boost/hana/tuple.hpp>

#include "serialization.h"
#include "macro.h"

namespace quintet {

class Object {
public:
  Object() = default;
  GEN_MOVE(Object, default);
  GEN_COPY(Object, default);

  template <class T>
  explicit Object(const T & x)
      : buffer(quintet::serialize<T>(x)) {}

  GEN_MUTABLE_HANDLE(buffer);

  template <class T>
  T as() const {
    using namespace boost::hana::literals;
    return deserialize<T>(buffer)[0_c];
  }

private:
  std::string buffer;

}; // class Object

} // namespace quintet

#endif //QUINTET_OBJECT_H
