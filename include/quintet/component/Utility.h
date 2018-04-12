#ifndef QUINTET_UTILITY_H
#define QUINTET_UTILITY_H

#include <future>
#include <chrono>
#include <memory>

#include <rpc/client.h>

// macro
namespace quintet {

#define GEN_DELETE_COPY(NAME) \
    NAME(const NAME &) = delete; \
    NAME & operator=(const NAME &) = delete; \

#define GEN_DELETE_MOVE(NAME) \
    NAME(NAME &&) = delete; \
    NAME & operator=(NAME &&) = delete;

} /* namespace quintet */


#endif //QUINTET_UTILITY_H
