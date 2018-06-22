#include "misc/FileIO.h"
#include "Serialization.h"
#include <fstream>

namespace quintet {
void File::checkVar(const std::string &name, quintet::File::file_type type) {
  if (vars.find(name) == vars.end())
    throw std::runtime_error("\"" + name + "\" has not registered");
  if (vars[name] != type)
    throw std::runtime_error("\"" + name + "\" type error");
}

std::string File::int2String(int x) {
  std::string buf;
  buf.resize(4);
  buf[0] = (x >> 0) & 0xff;
  buf[1] = (x >> 8) & 0xff;
  buf[2] = (x >> 16) & 0xff;
  buf[3] = (x >> 24) & 0xff;
  return buf;
}
}