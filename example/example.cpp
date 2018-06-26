#include "Interface.h"
#include "service/log/Global.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>
#include <mutex>

int main(int arg, char **argv) {
  auto &initializer = quintet::logging::Initializer::getInstance();
  initializer.addId("172.20.10.9:8000");
  initializer.init();

  int n;
  std::cin >> n;
  quintet::Interface inf;
  std::string path = std::string(CMAKE_SOURCE_DIR) +
      "/example/RaftConfig/RaftConfig" + std::to_string(n) + ".json";
  inf.Configure(path);

  std::mutex m;
  std::vector<int> vec;
  inf.bind("append", [&m, &vec](int a) -> void {
    std::lock_guard<std::mutex> lk(m);
    std::cout << a << std::endl;
    vec.push_back(a);
  });
  inf.Start();

  while(std::cin.get() != 's')
    ;
  for (int i = 0; i < 50; ++i)
    inf.call("append", i);
  while(std::cin.get() != '#')
    ;

  std::ofstream fout(std::to_string(n) + ".txt");
  for (auto & item : vec)
    fout << item << std::endl;

  inf.Shutdown();
  return 0;
}