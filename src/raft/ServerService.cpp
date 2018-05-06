#include "ServerService.h"
#include <thread>
#include <chrono>


void quintet::Committer::bindCommit(std::function<void(LogEntry)> f) {
    commit_ = std::move(f);
}

void quintet::Committer::commit(quintet::LogEntry log) {
    commit_(std::move(log));
}
