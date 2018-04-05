#ifndef QUINTET_SAMPLECONSENSUS_H
#define QUINTET_SAMPLECONSENSUS_H

#include <cstdint>
#include <string>
#include <future>
#include <vector>
#include <functional>
#include <chrono>
#include <random>
#include <memory>

#include <boost/any.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/signals2.hpp>

#include <rpc/server.h>
#include <rpc/client.h>

#include "Defs.h"
#include "RaftDefs.h"

// SampleConsensus: no PRC
namespace quintet {

class SampleConsensus {
public:
    using ServerId = Port;

    void AddLog(std::string opName, std::string args, PrmIdx prmIdx);

    void BindCommitter(std::function<void(std::string, std::string, ServerId, PrmIdx)> f);

    ServerId Local() const;

    void Configure(const std::string & filename);

    void run() {}

    void stop() {}

private:
    ServerId id;
    std::vector<ServerId> srvList;

    boost::signals2::signal<void(std::string, std::string, ServerId, PrmIdx)> commit;
    // dummy_mutex ??
}; // class SampleConsensus

} /* namespace quintet */


// TODO: SampleConsensusRPC
namespace quintet {

class SampleConsensusRPC {
public:
    using ServerId = Port;

    void AddLog(std::string opName, std::string args, PrmIdx prmIdx) {
        std::thread([this, opName = std::move(opName), args = std::move(args), prmIdx] {
            using namespace std::chrono_literals;
            std::default_random_engine generator;
            std::uniform_int_distribution<int> dist(0,100);
            std::this_thread::sleep_for(std::chrono::milliseconds(dist(generator)));
            commit(std::move(opName), std::move(args), id, prmIdx);
        }).detach();
    }

    void BindCommitter(std::function<void(std::string, std::string, ServerId, PrmIdx)> f) {
        commit.connect(f);
    }

    ServerId Local() const {
        return id;
    }

    void Configure(const std::string & filename) {
        namespace pt = boost::property_tree;

        pt::ptree tree{};
        pt::read_json(filename, tree);

        id = tree.get<ServerId>("Consensus.LocalId");
        srvList.clear();
        for (auto && item : tree.get_child("Consensus.ServerList")) {
            srvList.emplace_back(item.second.get_value<ServerId>());
        }
    }

    void run() {}

    void stop() {}

private:
    ServerId id;
    std::vector<ServerId> srvList;
    std::unique_ptr<rpc::server> srv;

    boost::signals2::signal<void(std::string, std::string, ServerId, PrmIdx)> commit;

};

} // namespace quintet

#endif //QUINTET_SAMPLECONSENSUS_H
