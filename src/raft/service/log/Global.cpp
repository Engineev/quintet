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

void quintet::logging::init() {
    static std::once_flag flag;
    std::call_once(flag, [] {
        logging::add_common_attributes();
        logging::core::get()->add_global_attribute("Scope", attrs::named_scope());
        logging::core::get()->add_sink(makeGlobalSink());
    });
}
