#ifndef QUINTET_UTILITY_H
#define QUINTET_UTILITY_H

#include <cstdint>
#include <random>

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

class Rand {
public:
    Rand(std::int64_t lb, std::int64_t up)
            : eng(std::random_device()()), dist(lb, up) {}

    std::int64_t operator()() {
        return dist(eng);
    }

private:
    std::default_random_engine eng;
    std::uniform_int_distribution<std::int64_t> dist;
}; // class Rand

} // namespace quintet


#endif //QUINTET_UTILITY_H
