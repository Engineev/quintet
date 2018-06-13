#ifndef QUINTET_IDENTITYTRANSFORMER_H
#define QUINTET_IDENTITYTRANSFORMER_H

#include <functional>
#include <memory>

#include "RaftDefs.h"

namespace quintet {

class IdentityTransformer {
public:
  IdentityTransformer();

  ~IdentityTransformer();

  void bind(std::function<void(quintet::ServerIdentityNo)> slot);

  bool notify(ServerIdentityNo target, Term currentTerm);

private:
  struct Impl;
  std::unique_ptr<Impl> pImpl;
};

} // namespace quintet

#endif // QUINTET_IDENTITYTRANSFORMER_H
