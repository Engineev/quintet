#include "server/config.h"

#include <functional>
#include "common/config.h"

namespace quintet {

struct Config::Impl {
  ServerInfo load(std::string filename) {
    ServerInfo res;
    res.load(filename);
    return res;
  }

  void save(ServerInfo info, std::string filename) {
    info.save(filename);
  }

}; /* struct Config::Impl */

} /* namespace quintet */

namespace quintet {

Config::Config() : pImpl(std::make_unique<Impl>()) {
  using namespace std::placeholders;

  bind<tag::LoadConfig>(std::bind(&Impl::load, &*pImpl, _1));
  bind<tag::SaveConfig>(std::bind(&Impl::save, &*pImpl, _1, _2));
}
GEN_PIMPL_DTOR(Config);

} /* namespace quintet */