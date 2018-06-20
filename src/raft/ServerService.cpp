#include "ServerService.h"

#include "service/log/Common.h"

namespace quintet {

void ServerService::configLogger(const std::string &id) {
  using namespace logging;
//  identityTransformer.configLogger(id);
  heartBeatController.configLogger(id);

  logger.add_attribute("ServerId", attrs::constant<std::string>(id));
}

} // namespace quintet