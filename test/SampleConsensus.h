#ifndef QUINTET_SAMPLECONSENSUS_H
#define QUINTET_SAMPLECONSENSUS_H

#include <cstdint>
#include <string>
#include <future>
#include <vector>
#include <functional>
#include <chrono>
#include <random>

#include <boost/any.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/signals2.hpp>

#include <rpc/server.h>
#include <rpc/client.h>

#include "quintet/Defs.h"

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

} // namespace quintet


// TODO: SampleConsensusRPC
namespace quintet {
} // namespace quintet

#endif //QUINTET_SAMPLECONSENSUS_H
