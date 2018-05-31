#include "Utility.h"

#include <random>

namespace quintet {

std::int64_t intRand(std::int64_t lb, std::int64_t ub) {
    static thread_local std::random_device rd;
    static thread_local std::mt19937 generator(rd());
    std::uniform_int_distribution<std::int64_t> distribution(lb, ub);
    return distribution(generator);
}

} // namespace quintet