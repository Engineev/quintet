#include "Interface.h"
#include "service/log/Global.h"
#include <iostream>
#include <string>

int main(int arg, char **argv) {
//  if (arg != 2) throw;
  int n;
  std::cin >> n;
  auto &initializer = quintet::logging::Initializer::getInstance();
  initializer.addId("127.0.0.1:50001");
  initializer.addId("127.0.0.2:50002");
  initializer.addId("127.0.0.3:50003");
  initializer.addId("localhost:8004");
  initializer.init();

  quintet::Interface inf;
  std::string path = std::string(CMAKE_SOURCE_DIR) +
      "/example/RaftConfig/RaftConfig" + std::to_string(n) +
      ".json";
  std::cerr << path << std::endl;
  inf.Configure(path);

  inf.bind("print", [](std::string s) -> void { std::cout << s << std::endl; });
  inf.Start();

  std::string s;
  while (std::cin >> s) {
    inf.call("print", s);
  }
  inf.Shutdown();
  return 0;
}