#ifndef QUINTET_SERVERINFO_H
#define QUINTET_SERVERINFO_H

#include <cstdint>
#include <utility>
#include <string>
#include <vector>
#include <iostream>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "quintet/Defs.h"
#include "quintet/Utility.h"

namespace quintet {

struct ServerId {
    std::string addr = "";
    Port port = 0;

    friend std::istream & operator>>(std::istream & in, ServerId & id);

    friend std::ostream & operator<<(std::ostream & out, const ServerId & id);

    friend bool operator==(const ServerId & lhs, const ServerId & rhs);

    GEN_NOT_EQUAL(ServerId);
};

struct ServerInfo {
    ServerId local;
    std::vector<ServerId> srvList;
    // TODO: timeout

    void load(const std::string & filename);

    void save(const std::string & filename);
};


} // namespace quintet



#endif //QUINTET_SERVERINFO_H
