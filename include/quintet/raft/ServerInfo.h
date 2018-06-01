#ifndef QUINTET_SERVERINFO_H
#define QUINTET_SERVERINFO_H

/**
 *  The identity and configuration of a server
 */

#include <cstdint>
#include <utility>
#include <string>
#include <vector>
#include <iostream>
#include <functional>

#include <rpc/server.h>

#include "RaftDefs.h"
#include "Utility.h"

// ServerId
namespace quintet {

struct ServerId {
    std::string addr = "";
    Port port = 0;

    std::string toString() const;

    MSGPACK_DEFINE_ARRAY(addr, port);
};

std::istream &operator>>(std::istream &in, ServerId &id);

std::ostream &operator<<(std::ostream &out, const ServerId &id);

bool operator==(const ServerId &lhs, const ServerId &rhs);

bool operator!=(const ServerId & lhs, const ServerId & rhs);

const ServerId NullServerId{"", 0};

} /* namespace quintet */

// hash
namespace std {
template <>
struct hash<quintet::ServerId> {
    std::size_t operator()(const quintet::ServerId& id) const {
        return std::hash<std::string>()(id.toString());
    }
};

} /* namespace std */

// ServerInfo
namespace quintet {

struct ServerInfo {
    ServerId local;
    std::vector<ServerId> srvList;
    std::uint64_t electionTimeout;
    // TODO: timeout

    void load(const std::string & filename);

    void save(const std::string & filename);
};

} // namespace quintet



#endif //QUINTET_SERVERINFO_H
