#include "log/Global.h"

#include <mutex>

#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/attributes/constant.hpp>

#include "log/Common.h"
#include "log/Sinks.h"

quintet::logging::Initializer &quintet::logging::Initializer::getInstance() {
    static Initializer inst;
    return inst;
}

void quintet::logging::Initializer::init() {
    logging::add_common_attributes();
    auto core = logging::core::get();
    auto sinks = makeGlobalSink(idList, prefix);
    for (auto & sink : sinks)
        core->add_sink(sink);
}

void quintet::logging::Initializer::addId(std::string id) {
    idList.emplace_back(std::move(id));
}

void quintet::logging::Initializer::setPrefix(std::string prefix_) {
    prefix = std::move(prefix_);
}
