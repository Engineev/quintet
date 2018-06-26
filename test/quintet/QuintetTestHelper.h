#ifndef QUINTET_QUINTETTESTHELPER_H
#define QUINTET_QUINTETTESTHELPER_H

#include <vector>

#include "Interface.h"

namespace quintet {
namespace test {

struct QuintetTestHelper {
  std::vector<quintet::Interface> makeInf(std::size_t num);

}; // class QuintetTestHelper

} // namespace test
} // namespace quintet

#endif //QUINTET_QUINTETTESTHELPER_H
