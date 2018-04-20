#ifndef QUINTET_IDENTITYTRANSFORMER_H
#define QUINTET_IDENTITYTRANSFORMER_H

#include <functional>
#include <thread>
#include <mutex>

#include "RaftDefs.h"

// IdentityTransformer
namespace quintet {

class IdentityTransformer {
public:
    /// \brief Bind the transformation slot. See param for details.
    ///
    /// \param slot Requirement: 'slot' should try to trigger a transformation
    ///             instead of actually carry out a transformation. That means
    ///             it should return immediately. Return whether the transformation
    ///             will be carried out.
    void bindNotificationSlot(std::function<bool(quintet::ServerIdentityNo)> slot) {
        notifySlot = std::move(slot);
    }

    /// \brief Notify the server to transform the identity. Return
    ///        immediately. The transformation is not guaranteed to
    ///        happen. Just send a notification.
    ///
    /// \param target The target identity
    /// \return Whether the server will carry out the transformation.
    bool notify(ServerIdentityNo target) {
        return notifySlot(target);
    }

private:
    std::function<bool(quintet::ServerIdentityNo)> notifySlot;
};

} /* namespace quintet */

#endif //QUINTET_IDENTITYTRANSFORMER_H
