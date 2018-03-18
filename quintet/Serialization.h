#ifndef QUINTET_SERIALIZATION_H
#define QUINTET_SERIALIZATION_H

#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/type.hpp>

/**
 * Serialization:
 * Args... args ==> std::string ==> hana::tuple
 *      ^                               |
 *      |========== hana::unpack =======|
 */

namespace quintet {

// forward declarations
template <class... Args>
std::string serialize(const Args&... args);

template <class... Args>
boost::hana::tuple<Args...> deserialize(std::string str);

namespace detail {
// serialize
inline void serialize_impl(boost::archive::text_oarchive & oa) {}

template <class T, class... Args>
void serialize_impl(boost::archive::text_oarchive & oa, const T & x, const Args&... args) {
    oa << x;
    serialize_impl(oa, args...);
};

} // namespace detail

template <class... Args>
std::string serialize(const Args&... args) {
    std::stringstream ss;
    {
        boost::archive::text_oarchive oa(ss);
        detail::serialize_impl(oa, args...);
    }
    return std::string{std::istreambuf_iterator<char>(ss), {}};
}

template <class... Args>
boost::hana::tuple<Args...> deserialize(std::string str) {
    boost::hana::tuple<Args...> res;
    std::stringstream ss(std::move(str));
    boost::archive::text_iarchive ia(ss);
    boost::hana::for_each(res, [&](auto & element) {
        ia >> element;
    });
    return res;
}

} // namespace quintet

#endif //QUINTET_SERIALIZATION_H
