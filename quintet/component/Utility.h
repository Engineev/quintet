#ifndef QUINTET_UTILITY_H
#define QUINTET_UTILITY_H

#include <future>
#include <chrono>
#include <memory>

#include <rpc/client.h>

// macro
namespace quintet {

#define GEN_NOT_EQUAL(NAME) \
    bool operator!=(const NAME & lhs, const NAME & rhs) { \
        return !(lhs == rhs); \
    }

#define GEN_DELETE_COPY(NAME) \
    NAME(const NAME &) = delete; \
    NAME & operator=(const NAME &) = delete; \

#define GEN_DELETE_MOVE(NAME) \
    NAME(NAME &&) = delete; \
    NAME & operator=(NAME &&) = delete;

} /* namespace quintet */


#endif //QUINTET_UTILITY_H
