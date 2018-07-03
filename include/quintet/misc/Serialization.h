#ifndef QUINTET_SERIALIZATION_H
#define QUINTET_SERIALIZATION_H

#include <sstream>
#include <stdexcept>

#include <iostream>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/type.hpp>
#include <boost/serialization/vector.hpp>

/**
 * Serialization:
 * Args... args ==> std::string ==> hana::tuple
 *      ^                               |
 *      |========== hana::unpack =======|
 */

namespace quintet {

class TypeMismatched : public std::exception {};

// forward declarations
template <class... Args> std::string serialize(const Args &... args);

template <class... Args>
boost::hana::tuple<Args...> deserialize(std::string str);

namespace {
// serialize
inline void serialize_impl(boost::archive::text_oarchive &oa) {}

template <class T, class... Args>
void serialize_impl(boost::archive::text_oarchive &oa, const T &x,
                    const Args &... args) {
  oa << x;
  serialize_impl(oa, args...);
};

template <class T>
void serializeTypeInfo(std::vector<std::size_t> & types) {
  types.emplace_back(typeid(T).hash_code());
}

template <class T, class U, class... Args>
void serializeTypeInfo(std::vector<std::size_t> & types) {
  types.emplace_back(typeid(T).hash_code());
  serializeTypeInfo<U, Args...>(types);
}

} // namespace

template <class... Args> std::string serialize(const Args &... args) {
//  std::vector<std::size_t> types;
//  serializeTypeInfo<Args...>(types);

  std::stringstream ss;
  {
    boost::archive::text_oarchive oa(ss);
//    serialize_impl(oa, types, args...);
    serialize_impl(oa, args...);
  }
  return std::string{std::istreambuf_iterator<char>(ss), {}};
}

template <class... Args>
boost::hana::tuple<Args...> deserialize(std::string str) {
  using namespace boost::hana;
  tuple<Args...> res;
  std::stringstream ss(str);
  boost::archive::text_iarchive ia(ss);

//  std::vector<std::size_t> typesSerialized;
//  ia >> typesSerialized;

//  std::vector<std::size_t> types;
//  serializeTypeInfo<Args...>(types);

//  if (types != typesSerialized)
//    throw TypeMismatched();

  for_each(res, [&](auto & element) { ia >> element; });

  return res;
}

} // namespace quintet

#endif // QUINTET_SERIALIZATION_H
