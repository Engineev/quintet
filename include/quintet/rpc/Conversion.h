#ifndef QUINTET_CONVERSION_H
#define QUINTET_CONVERSION_H

#include "Quintet.pb.h"
#include "common.h"

// Common -> Pb
namespace quintet {
namespace rpc {

std::string convertServerId(const ServerId & srv);


} // namespace rpc
} // namespace quintet


// Pb -> Common
namespace quintet {
namespace rpc {

ServerId convertServerId(const std::string & srv);

} // namespace rpc
} // namespace quintet

#endif //QUINTET_CONVERSION_H
