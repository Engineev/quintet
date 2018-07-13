#include "common.h"

namespace quintet {

ServerId::ServerId(std::string addr, Port port)
    : addr(std::move(addr)), port(port) {}

ServerId::ServerId(const std::string & exAddr) {
  auto sepPos = exAddr.find(':');
  addr = std::string(exAddr.begin(), exAddr.begin() + sepPos);
  port = std::stoi(std::string(exAddr.cbegin() + sepPos + 1, exAddr.end()));
}

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