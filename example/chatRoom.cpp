#include "Interface.h"
#include "service/log/Global.h"
#include <iostream>
#include <string>

int main(int arg, char **argv) {
  if (arg != 2) throw;
  auto &initializer = quintet::logging::Initializer::getInstance();
  initializer.addId("127.0.0.1:50001");
  initializer.addId("127.0.0.2:50002");
  initializer.init();

  quintet::Interface inf;
  std::string path = std::string(CMAKE_SOURCE_DIR) +
      "/example/RaftConfig/RaftConfig" + std::string(argv[1]) +
      ".json";
  //std::cerr << path << std::endl;
  inf.Configure(path);

  inf.bind("print", [](std::string id, std::string s) -> void { 
    std::cout << "< " << id << " >: " << s << std::endl; });
  inf.Start();

  std::string s;
  while (std::cin >> s) {
    inf.call("print", std::string(argv[1]), s);
  }
  inf.Shutdown();
  return 0;
}