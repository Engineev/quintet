#pragma once

#include <boost/log/sources/logger.hpp>

#include "service/IdentityTransformer.h"
#include "service/HeartBeatController.h"
#include "service/rpc/RpcClients.h"
//#include "service/log/Common.h"
// Do NOT include log/Common.h here since this file will be included in many
// headers and including log/Common.h will increase the compile time.

namespace quintet {

struct ServerService {
  HeartBeatController heartBeatController;
  IdentityTransformer identityTransformer;
//  rpc::RpcClients     clients;
  boost::log::sources::logger_mt logger;

  void configLogger(const std::string &id);

}; // struct ServerService

} // namespace quintet