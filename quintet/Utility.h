#ifndef QUINTET_UTILITY_H
#define QUINTET_UTILITY_H

namespace quintet {

#define GEN_NOT_EQUAL(NAME) \
    friend bool operator!=(const NAME & lhs, const NAME & rhs) { \
        return !(lhs == rhs); \
    }

#define GEN_DELETE_COPY_AND_MOVE(NAME) \
    NAME(const NAME &) = delete; \
    NAME(NAME &&) = delete; \
    NAME & operator=(const NAME &) = delete; \
    NAME & operator=(NAME &&) = delete;



}

#endif //QUINTET_UTILITY_H
