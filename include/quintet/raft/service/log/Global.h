#ifndef QUINTET_LOGGING_H
#define QUINTET_LOGGING_H

#include <vector>
#include <string>

namespace quintet {
namespace logging {

class Initializer {
public:
  static Initializer & getInstance();

  void setPrefix(std::string prefix_);

  void addId(std::string id);

  void init();

private:
  std::string prefix = "./";
  std::vector<std::string> idList;

}; // class Initializer


} // namespace logging
} // namespace quintet

#endif //QUINTET_LOGGING_H
