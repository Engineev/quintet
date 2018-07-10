#include "common.h"

namespace quintet {

ServerId::ServerId(std::string addr, Port port)
    : addr(std::move(addr)), port(port) {}

std::string ServerId::toString(std::string sep) const {
  return addr + sep + std::to_string(port);
}

void ServerId::clear() {
  addr.clear();
  port = 0;
}

const bool ServerId::empty() const {
  return addr.empty() && port == 0;
}

} // namespace quintet