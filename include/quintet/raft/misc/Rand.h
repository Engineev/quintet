#ifndef QUINTET_UTILITY_H
#define QUINTET_UTILITY_H

#include <cstdint>

namespace quintet {

/// thread safe random number
std::int64_t intRand(std::int64_t lb, std::int64_t ub);

} // namespace quintet

#endif //QUINTET_UTILITY_H
