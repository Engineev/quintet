#include "Interface.h"

namespace quintet {

Interface::Interface() : pImpl(std::make_unique<Impl>()) {}

void Interface::Impl::addLog(const std::string &name, const std::string &args,
                             const PrmIdx prmIdx) {
  bool success = false;
  ServerId leaderId;
}

} // namespace quintet