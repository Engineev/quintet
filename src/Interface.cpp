#include "Interface.h"

namespace quintet {

Interface::Interface() : pImpl(std::make_unique<Impl>()) {}


} // namespace quintet