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

template <class T, class Item = std::unique_ptr<rpc::client>>
class FutureWrapper {
public:
    FutureWrapper(boost::future<T> && fut, Item && item) noexcept
            : fut_(std::move(fut)), item_(std::move(item)) {}

    FutureWrapper(FutureWrapper && o) noexcept
            : fut_(std::move(o.fut_)), item_(std::move(o.item_)) {}

    FutureWrapper & operator=(FutureWrapper && o) noexcept {
        if (this == &o)
            return *this;
        fut_  = std::move(o.fut_);
        item_ = std::move(o.item_);
        return *this;
    }

    T get() {
        return fut_.get();
    }

    bool ready() const {
        return fut_.wait_for(boost::chrono::seconds(0)) == boost::future_status::ready;
    }

private:
    boost::future<T> fut_;
    Item item_;
};

}

#endif //QUINTET_FUTURE_H
