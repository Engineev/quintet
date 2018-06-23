#ifndef QUINTET_STATEINTERFACE_H
#define QUINTET_STATEINTERFACE_H

#include <functional>
#include <memory>
#include <vector>

#include "ServerState.h"
#include "ServerInfo.h"
#include "service/rpc/RpcDefs.h"

namespace quintet {

struct StateInterfaceImpl;

class StateInterface {
public:
  virtual ~StateInterface();

protected:
  std::shared_ptr<StateInterfaceImpl> pImpl;
}; // class StateInterface

} // namespace quintet

#endif //QUINTET_STATEINTERFACE_H
