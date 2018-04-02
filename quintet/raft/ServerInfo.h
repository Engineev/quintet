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

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <rpc/server.h>

#include "quintet/raft/RaftDefs.h"
#include "quintet/component/Utility.h"

namespace quintet {

struct ServerId {
    std::string addr = "";
    Port port = 0;

    MSGPACK_DEFINE_ARRAY(addr, port);

    friend std::istream & operator>>(std::istream & in, ServerId & id);

    friend std::ostream & operator<<(std::ostream & out, const ServerId & id);

    friend bool operator==(const ServerId & lhs, const ServerId & rhs);

    GEN_NOT_EQUAL(ServerId);
};

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
