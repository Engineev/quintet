#ifndef QUINTET_SINKS_H
#define QUINTET_SINKS_H

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include "Common.h"

namespace quintet {
namespace logging {

using Sink = sinks::synchronous_sink<sinks::text_file_backend>;

std::vector<boost::shared_ptr<Sink>> makeGlobalSink(const std::vector<std::string> & idList, const std::string & prefix);



} // namespace logging
} // namespace quintet

#endif //QUINTET_SINKS_H
