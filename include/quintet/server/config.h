#pragma once

#include <memory>
#include "actor_types.h"

namespace quintet {

/**
 * This module maintains the information about the cluster and the server
 */

class Config : public ConfigActor {
public:
  Config();
  ~Config();

private:
  GEN_PIMPL_DEF();
};

} // namespace quintet

