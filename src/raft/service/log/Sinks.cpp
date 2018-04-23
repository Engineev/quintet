#include "log/Sinks.h"

#include <iomanip>

#include <boost/make_shared.hpp>
#include <boost/log/support/date_time.hpp>

std::vector<boost::shared_ptr<quintet::logging::Sink>>
quintet::logging::makeGlobalSink(
        const std::vector<std::string> &idList, const std::string & prefix) {
    std::vector<std::string> fileNames;
    for (auto & id : idList)
        fileNames.emplace_back(prefix + id + "_%3N" + ".log");

    auto makeBackEnd = [&](std::size_t idx) {
        auto backEnd = boost::make_shared<sinks::text_file_backend>(
                keywords::file_name = fileNames[idx],
                keywords::rotation_size = 5 * 1024 * 1024,
                keywords::auto_flush = true
        );
        return backEnd;
    };
    auto makeFrontEnd = [&](std::size_t idx, boost::shared_ptr<sinks::text_file_backend> backend) {
        auto frontEnd = boost::make_shared<Sink>(backend);
        frontEnd->set_filter(expr::has_attr("ServerId") && expr::attr<std::string>("ServerId") == idList[idx]);
        frontEnd->set_formatter(
            expr::stream
                << "[" << expr::format_date_time<boost::posix_time::ptime>(
                    "TimeStamp", "%Y-%m-%d %H:%M:%S.%f") << "] "
                << expr::if_(expr::has_attr<std::string>("ServiceType"))[
                    expr::stream << "[" << std::setw(11) << expr::attr<std::string>("ServiceType") << "] "
                ].else_[
                    expr::stream << "[" << std::setw(11) << " " << "] "
                ]
                << expr::smessage
        );
        return frontEnd;
    };

    std::vector<boost::shared_ptr<Sink>> res;
    for (std::size_t i = 0; i < idList.size(); ++i)
        res.emplace_back(makeFrontEnd(i, makeBackEnd(i)));
    return res;
}
