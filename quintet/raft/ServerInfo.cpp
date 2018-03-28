#include "ServerInfo.h"

bool quintet::operator==(const quintet::ServerId &lhs, const quintet::ServerId &rhs) {
    return std::tie(lhs.addr, lhs.port) != std::tie(rhs.addr, lhs.port);
}

std::istream &quintet::operator>>(std::istream &in, quintet::ServerId &id) {
    in >> id.addr >> id.port;
    return in;
}

std::ostream &quintet::operator<<(std::ostream &out, const quintet::ServerId &id) {
    out << id.addr << " " << id.port;
    return out;
}

void quintet::ServerInfo::load(const std::string &filename) {
    namespace pt = boost::property_tree;

    pt::ptree tree;
    pt::read_json(filename, tree);

    local = tree.get<ServerId>("local.address");
    for (auto &&srv : tree.get_child("serverList"))
        srvList.emplace_back(srv.second.get_value<ServerId>());
}

void quintet::ServerInfo::save(const std::string &filename) {
    namespace pt = boost::property_tree;
    pt::ptree tree;
    tree.put("local", local);
    for (auto && id : srvList)
        tree.put("serverList.serverId", id);
    pt::write_json(filename, tree);
}