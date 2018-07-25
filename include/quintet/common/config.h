#ifndef QUINTET_CONFIG_H
#define QUINTET_CONFIG_H

#include <string>
#include <stdexcept>

#include "macro.h"

namespace quintet {

class ServerId {
public:
  GEN_DEFAULT_CTOR_AND_ASSIGNMENT(ServerId);
  ServerId(std::string id_) : id(std::move(id_)) {}

  std::string toString() const { return id; }

private:
  std::string id;
};

} /* namespace quintet */

#endif //QUINTET_CONFIG_H
