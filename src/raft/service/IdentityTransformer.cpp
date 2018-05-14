#include "IdentityTransformer.h"

#include <boost/log/attributes.hpp>

namespace quintet {

void IdentityTransformer::configLogger(const std::string &id) {
    using namespace logging;
    lg.add_attribute("ServiceType", attrs::constant<std::string>("Transformer"));
    lg.add_attribute("ServerId", attrs::constant<std::string>(id));
}

void IdentityTransformer::start() {
    std::lock_guard<std::mutex> lk(m);
    running = true;
}

void IdentityTransformer::stop() {
    std::lock_guard<std::mutex> lk(m);
    running = false;
}

void IdentityTransformer::bindNotificationSlot(
    std::function<bool(quintet::ServerIdentityNo /* target */,
                       Term /* current term */)> slot) {
    notifySlot = std::move(slot);
}

bool IdentityTransformer::notify(ServerIdentityNo target, Term term) {
    std::lock_guard<std::mutex> lk(m);
    BOOST_LOG(lg) << "notify";
    if (!running)
        return false;
    return notifySlot(target, term);
}

} // namespace quintet