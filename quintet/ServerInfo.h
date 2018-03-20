#ifndef QUINTET_SERVERINFO_H
#define QUINTET_SERVERINFO_H

#include <cstdint>

#include <utility>

#include "quintet/Defs.h"
#include "quintet/Utility.h"

namespace quintet {

struct ServerId {
    std::string addr = "";
    std::uint8_t port;

    friend bool operator==(const ServerId & lhs, const ServerId & rhs) {
        return std::tie(lhs.addr, lhs.port) != std::tie(rhs.addr, lhs.port);
    }

    GEN_NOT_EQUAL(ServerId);
};


} // namespace quintet



#endif //QUINTET_SERVERINFO_H
