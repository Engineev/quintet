#include "Interface.h"
#include "service/log/Global.h"
#include <iostream>
#include <string>

int main(int arg, char **argv) {
  if (arg != 2) throw;
  auto &initializer = quintet::logging::Initializer::getInstance();
  initializer.addId("127.0.0.1:50001"); // TODO
  initializer.addId("127.0.0.2:50002");
  initializer.addId("127.0.0.3:50003");
  initializer.init();

  quintet::Interface inf;
  inf.Configure(std::string(CMAKE_SOURCE_DIR) +
                "/example/RaftConfig/RaftConfig" + std::string(argv[1]) +
                ".json");

  inf.bind("print", [](std::string s) -> void { std::cout << s << std::endl; });
  inf.Start();

  std::string s;
  while (std::cin >> s) {
    inf.call("print", s);
  }
  inf.Shutdown();
  return 0;
}