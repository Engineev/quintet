#include "server_info.h"

#include <iostream>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace quintet {

std::istream & operator>>(std::istream &in, ServerId & id) {
  std::string addr;
  Port port;
  in >> addr >> port;
  id = ServerId(addr, port);
  return in;
}

std::ostream & operator<<(std::ostream &out, const ServerId &id) {
  out << id.get_addr() << id.get_port();
  return out;
}


void ServerInfo::load(const std::string &filename) {
  namespace pt = boost::property_tree;

  pt::ptree tree;
  pt::read_json(filename, tree);

  local = tree.get<ServerId>("local.address");
  electionTimeout = tree.get<std::uint64_t>("electionTimeout");
  for (auto &&srv : tree.get_child("serverList"))
    srvList.emplace_back(srv.second.get_value<ServerId>());
}

void ServerInfo::save(const std::string &filename) {
  namespace pt = boost::property_tree;
  pt::ptree tree;
  tree.put("local.address", local);
  tree.put("electionTimeout", electionTimeout);
  for (auto &&id : srvList)
    tree.put("serverList.serverId", id);
  pt::write_json(filename, tree);
}

} // namespace quintet