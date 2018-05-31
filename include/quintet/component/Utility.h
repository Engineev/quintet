#ifndef QUINTET_UTILITY_H
#define QUINTET_UTILITY_H

#include <cstdint>

// macro
namespace quintet {

#define GEN_DELETE_COPY(NAME) \
    NAME(const NAME &) = delete; \
    NAME & operator=(const NAME &) = delete; \

#define GEN_DELETE_MOVE(NAME) \
    NAME(NAME &&) = delete; \
    NAME & operator=(NAME &&) = delete;
} /* namespace quintet */

// random
namespace quintet {

std::int64_t intRand(std::int64_t lb, std::int64_t ub);

} // namespace quintet

#endif //QUINTET_UTILITY_H
