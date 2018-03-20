#ifndef QUINTET_UTILITY_H
#define QUINTET_UTILITY_H

namespace quintet {

#define GEN_NOT_EQUAL(NAME) \
        friend bool operator!=(const NAME & lhs, const NAME & rhs) { \
            return !(lhs == rhs); \
        }

}

#endif //QUINTET_UTILITY_H
