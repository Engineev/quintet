#include "SampleConsensus.h"

#include <iostream>

quintet::SampleConsensus::ServerId quintet::SampleConsensus::Local() const {
    return id;
}

void quintet::SampleConsensus::Configure(const std::string &filename) {
    namespace pt = boost::property_tree;

    pt::ptree tree{};
    pt::read_json(filename, tree);

    id = tree.get<ServerId>("Consensus.LocalId");
    srvList.clear();
    for (auto && item : tree.get_child("Consensus.ServerList")) {
        srvList.emplace_back(item.second.get_value<ServerId>());
    }
}

void quintet::SampleConsensus::BindCommitter(
        std::function<void(std::string, std::string, quintet::SampleConsensus::ServerId, quintet::PrmIdx)> f) {
    commit.connect(f);
}

void quintet::SampleConsensus::AddLog(std::string opName, std::string args, quintet::PrmIdx prmIdx) {
    std::thread([this, opName = std::move(opName), args = std::move(args), prmIdx] {
        using namespace std::chrono_literals;
        std::default_random_engine generator;
        std::uniform_int_distribution<int> dist(0,100);
        std::this_thread::sleep_for(std::chrono::milliseconds(dist(generator)));
        commit(std::move(opName), std::move(args), id, prmIdx);
    }).detach();
}
