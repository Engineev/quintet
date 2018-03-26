#ifndef QUINTET_UTILITY_H
#define QUINTET_UTILITY_H

#include <future>

// macro
namespace quintet {

#define GEN_NOT_EQUAL(NAME) \
    friend bool operator!=(const NAME & lhs, const NAME & rhs) { \
        return !(lhs == rhs); \
    }

#define GEN_DELETE_COPY(NAME) \
    NAME(const NAME &) = delete; \
    NAME & operator=(const NAME &) = delete; \

#define GEN_DELETE_MOVE(NAME) \
    NAME(NAME &&) = delete; \
    NAME & operator=(NAME &&) = delete;

} /* namespace quintet */

// Future wrapper
namespace quintet {

template <class T, class Item>
class FutureWrapper {
public:
    explicit FutureWrapper(std::future<T> && fut, Item && item) noexcept
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

    operator std::future<T>& () {
        return fut_;
    }

private:
    std::future<T> fut_;
    Item item_;
};

} /* namespace quintet */

#endif //QUINTET_UTILITY_H
