#include "ServerInfo.h"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

bool quintet::operator==(const quintet::ServerId &lhs, const quintet::ServerId &rhs) {
    return std::tie(lhs.addr, lhs.port) == std::tie(rhs.addr, rhs.port);
}

bool quintet::operator!=(const quintet::ServerId &lhs, const quintet::ServerId &rhs) {
    return !(lhs == rhs);
}


std::istream & quintet::operator>>(std::istream &in, quintet::ServerId &id) {
    in >> id.addr >> id.port;
    return in;
}

std::ostream & quintet::operator<<(std::ostream &out, const quintet::ServerId &id) {
    out << id.addr << " " << id.port;
    return out;
}

void quintet::ServerInfo::load(const std::string &filename) {
    namespace pt = boost::property_tree;

    pt::ptree tree;
    pt::read_json(filename, tree);

    local = tree.get<ServerId>("local.address");
    electionTimeout = tree.get<std::uint64_t>("electionTimeout");
    for (auto &&srv : tree.get_child("serverList"))
        srvList.emplace_back(srv.second.get_value<ServerId>());
}

void quintet::ServerInfo::save(const std::string &filename) {
    namespace pt = boost::property_tree;
    pt::ptree tree;
    tree.put("local.address", local);
    tree.put("electionTimeout", electionTimeout);
    for (auto && id : srvList)
        tree.put("serverList.serverId", id);
    pt::write_json(filename, tree);
}
