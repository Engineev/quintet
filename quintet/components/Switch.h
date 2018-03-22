#ifndef QUINTET_SWITCH_H
#define QUINTET_SWITCH_H

/**
 *  A switch can be turned off, turned on and held so that
 *  the status of it can not be changed.
 *  turnOn & turnOff will return the original status.
 */

#include <thread>
#include <mutex>

#include "quintet/Utility.h"

namespace quintet {

class Switch {
public:
    Switch() = default;

    GEN_DELETE_COPY_AND_MOVE(Switch)

    bool turnOn() {
        return setFlag(true);
    }

    bool turnOff() {
        return setFlag(false);
    }

    /*no_discard*/ std::unique_lock<std::mutex> hold() {
        return std::unique_lock<std::mutex>(m);
    }

private:
    bool flag = false;
    std::mutex m;

    bool setFlag(bool f) {
        std::lock_guard<std::mutex> lk(m);
        auto tmp = flag;
        flag = f;
        return tmp;
    }

}; // class Switch

} // namespace quintet


#endif //QUINTET_SWITCH_H
