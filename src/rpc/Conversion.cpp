#include "rpc/conversion.h"

// Common -> Pb
namespace quintet {
namespace rpc {

std::string convertServerId(const ServerId & srv) {
  return srv.toString();
}

} // namespace rpc
} // namespace quintet

// Pb -> Common
namespace quintet {
namespace rpc {

ServerId convertServerId(const std::string & srv) {
  auto sep = srv.cbegin() + srv.find(':');
  std::string addr{srv.cbegin(), sep};
  Port port = std::stoi(std::string(sep + 1, srv.end()), nullptr);
  return {addr, port};
}

} // namespace rpc
} // namespace quintet


