#ifndef QUINTET_COMMON_H
#define QUINTET_COMMON_H

#include <string>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/keyword_fwd.hpp>
#include <boost/log/expressions/keyword.hpp>

namespace quintet {
namespace logging {

namespace src      = boost::log::sources;
namespace logging  = boost::log;
namespace keywords = boost::log::keywords;
namespace sinks    = boost::log::sinks;
namespace attrs    = boost::log::attributes;
namespace expr     = boost::log::expressions;

} // namespace logging
} // namespace quintet

#endif //QUINTET_COMMON_H
