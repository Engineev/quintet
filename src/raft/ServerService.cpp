#include "ServerService.h"
#include <thread>
#include <chrono>

#include "log/Common.h"

void quintet::Committer::bindCommit(std::function<void(LogEntry)> f) {
    commit_ = std::move(f);
}

void quintet::Committer::commit(quintet::LogEntry log) {
    commit_(std::move(log));
}

// ServerService
namespace quintet {

void ServerService::configLogger(const std::string &id) {
    using namespace logging;
    identityTransformer.configLogger(id);
    heartBeatController.configLogger(id);

    logger.add_attribute("ServerId", attrs::constant<std::string>(id));
}

} /* namespace quintet */
