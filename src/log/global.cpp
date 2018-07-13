#include "log/global.h"

#include <iomanip>
#include <iostream>

#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

#include "common.h"
#include "log/common.h"
#include "raft/raft_common.h"

namespace quintet {
namespace logging {

namespace {
using Sink = sinks::synchronous_sink<sinks::text_file_backend>;
}

std::vector<boost::shared_ptr<Sink>>
makeGlobalSink(const std::vector<std::string> &idList,
               const std::string &prefix) {
  std::vector<std::string> fileNames;
  for (auto &id : idList)
    fileNames.emplace_back(prefix + id + "_%3N" + ".log");

  auto makeBackEnd = [&](std::size_t idx) {
    auto backEnd = boost::make_shared<sinks::text_file_backend>(
        keywords::file_name = fileNames[idx],
        keywords::rotation_size = 5 * 1024 * 1024, keywords::auto_flush = true);
    return backEnd;
  };
  auto makeFrontEnd = [&](std::size_t idx,
                          boost::shared_ptr<sinks::text_file_backend> backend) {
    auto frontEnd = boost::make_shared<Sink>(backend);
    frontEnd->set_filter(expr::has_attr("ServerId") &&
        expr::attr<std::string>("ServerId") == idList[idx]);
    // clang-format off
    frontEnd->set_formatter(
        expr::stream
            << "["
            << expr::format_date_time<boost::posix_time::ptime>(
                "TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
            << "] "
//            << expr::if_(expr::has_attr<std::string>("Part"))[
//                expr::stream << "[" << std::setw(11)
//                             << expr::attr<std::string>("Part") << "] "
//            ].else_[
//                expr::stream << "[" << std::setw(11) << " " << "] "
//            ]
            << expr::if_(expr::has_attr<int>("Identity"))[
                expr::stream << "[(Identity)"
                             << expr::attr<int>("Identity") << "] "
            ].else_[
                expr::stream << "[ ] "
            ]
            << expr::smessage);
    // clang-format on
    return frontEnd;
  };

  std::vector<boost::shared_ptr<Sink>> res;
  for (std::size_t i = 0; i < idList.size(); ++i)
    res.emplace_back(makeFrontEnd(i, makeBackEnd(i)));
  return res;
}

} // namespace logging
} // namespace quintet

namespace quintet {
namespace logging {

Initializer &Initializer::getInstance() {
  static Initializer inst;
  return inst;
}

void Initializer::init() {
  logging::add_common_attributes();
  logging::core::get()->add_global_attribute("Scope", attrs::named_scope());

  auto core = logging::core::get();
  auto sinks = makeGlobalSink(idList, prefix);
  for (auto &sink : sinks)
    core->add_sink(sink);
}

void Initializer::addId(std::string id) { idList.emplace_back(std::move(id)); }

void Initializer::setPrefix(std::string prefix_) {
  prefix = std::move(prefix_);
}

} // namespace logging
} // namespace quintet