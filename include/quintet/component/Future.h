#ifndef QUINTET_FUTURE_H
#define QUINTET_FUTURE_H

#include <future>
#include <exception>

#include <boost/thread/thread.hpp>
#include <boost/thread/future.hpp>

#include <rpc/server.h>

namespace quintet {

template <class T>
boost::future<T> toBoostFuture(std::future<T> && fut) {
    boost::promise<T> prm;
    auto res = prm.get_future();

    boost::thread(
            [prm = std::move(prm), fut = std::move(fut)]() mutable {
                try {
                    prm.set_value(fut.get());
                } catch (const std::exception & e) {
                    prm.set_exception(e);
                }
            }
    ).detach();

    return res;
}



}

#endif //QUINTET_FUTURE_H
