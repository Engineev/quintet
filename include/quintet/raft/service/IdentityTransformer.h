#ifndef QUINTET_IDENTITYTRANSFORMER_H
#define QUINTET_IDENTITYTRANSFORMER_H

#include <functional>
#include <thread>
#include <mutex>
#include <atomic>

#include "RaftDefs.h"
#include "log/Common.h"
#include <boost/log/attributes.hpp>

// IdentityTransformer
namespace quintet {

class IdentityTransformer {
public:
    /// \breif Configure the logger. This function should be called before
    ///        other functions being called.
    void configLogger(const std::string &id) {
        using namespace logging;
        lg.add_attribute("ServiceType", attrs::constant<std::string>("Transformer"));
        lg.add_attribute("ServerId", attrs::constant<std::string>(id));
    }

    void start() {
        std::lock_guard<std::mutex> lk(m);
        running = true;
    }

    void stop() {
        std::lock_guard<std::mutex> lk(m);
        running = false;
    }

    /// \brief Bind the transformation slot. See param for details.
    ///
    /// \param slot Requirement: 'slot' should try to trigger a transformation
    ///             instead of actually carry out a transformation. That means
    ///             it should return immediately. Return whether the transformation
    ///             will be carried out.
    void bindNotificationSlot(
            std::function<bool(quintet::ServerIdentityNo /* target */,
                               Term /* current term */)> slot) {
        notifySlot = std::move(slot);
    }

    /// \brief Notify the server to transform the identity. Return
    ///        immediately. The transformation is not guaranteed to
    ///        happen. Just send a notification.
    ///
    /// \param target The target identity
    /// \param term   Current term
    /// \return Whether the server will carry out the transformation.
    ///         If the transformer has been stopped, return false too.
    bool notify(ServerIdentityNo target, Term term) {
        std::lock_guard<std::mutex> lk(m);
        BOOST_LOG(lg) << "notify";
        if (!running)
            return false;
        return notifySlot(target, term);
    }

private:
    std::function<bool(quintet::ServerIdentityNo, Term)> notifySlot;
    std::mutex m;
    bool running = false;

    logging::src::logger_mt lg;
};

} /* namespace quintet */

#endif //QUINTET_IDENTITYTRANSFORMER_H
