#include "QuintetTestHelper.h"

namespace quintet {
namespace test {
std::vector<quintet::Interface> QuintetTestHelper::makeInf(std::size_t num) {
  assert(num <= 5);
  std::vector<Interface> res;
  for (int i = 0; i < (int)num; ++i) {
    Interface inf;
    inf.Configure(std::string(CMAKE_SOURCE_DIR) +
        "/test/RaftConfig/RaftConfig" + std::to_string(i) + ".json");
    res.emplace_back(std::move(inf));
  }
  return res;
}


}
}