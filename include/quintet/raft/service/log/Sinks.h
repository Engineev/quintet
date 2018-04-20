#ifndef QUINTET_SINKS_H
#define QUINTET_SINKS_H

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>

#include "Common.h"

namespace quintet {
namespace logging {

using Sink = sinks::synchronous_sink<sinks::text_file_backend>;

boost::shared_ptr<Sink> makeGlobalSink();


} // namespace logging
} // namespace quintet

#endif //QUINTET_SINKS_H
