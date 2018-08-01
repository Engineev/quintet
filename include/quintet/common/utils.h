#ifndef QUINTET_UTILS_H
#define QUINTET_UTILS_H

#include <cstdint>
#include <random>

namespace quintet {

template <class T>
T intRand(T lb, T ub) {
  static thread_local std::random_device rd;
  static thread_local std::mt19937 generator(rd());
  std::uniform_int_distribution<T> distribution(lb, ub);
  return distribution(generator);
}

} // namespace quintet

#endif //QUINTET_UTILS_H
