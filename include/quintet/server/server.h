#ifndef QUINTET_SERVER_H
#define QUINTET_SERVER_H

#include <string>
#include <memory>

namespace quintet {

class Server {
public:
  Server(const std::string & filename);
  ~Server();

  void start();

  void shutdown();

  void wait();

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;

}; /* class Server */

} /* namespace quintet */

#endif //QUINTET_SERVER_H
