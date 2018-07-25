#ifndef QUINTET_COMMON_H
#define QUINTET_COMMON_H

#include "common/config.h"

namespace quintet {

enum class IdentityNo { Follower = 0, Candidate, Leader, Down, Error };

} /* namespace quintet */

#endif //QUINTET_COMMON_H
