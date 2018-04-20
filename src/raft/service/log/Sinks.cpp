#include "log/Sinks.h"

boost::shared_ptr<quintet::logging::Sink> quintet::logging::makeGlobalSink() {
    boost::shared_ptr<sinks::text_file_backend> backend =
            boost::make_shared<sinks::text_file_backend>(
                    keywords::file_name = "file_%5N.log",
                    keywords::rotation_size = 5 * 1024 * 1024,
                    keywords::time_based_rotation = sinks::file::rotation_at_time_point(12, 0, 0)
            );
    auto sink = boost::make_shared<Sink>(backend);
    return sink;
}
