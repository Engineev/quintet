#include "RaftDefs.h"

namespace quintet {

std::string ServerId::toString() const {
  return addr + "-" + std::to_string(port);
}

bool operator==(const ServerId &lhs, const ServerId &rhs) {
  return std::tie(lhs.addr, lhs.port) == std::tie(rhs.addr, rhs.port);
}

bool operator!=(const ServerId &lhs, const ServerId &rhs) {
  return !(lhs == rhs);
}

} // namespace quintet