#ifndef CONCERTO_MACRO_H
#define CONCERTO_MACRO_H

#define GEN_PIMPL_CTOR(NAME)                                                   \
  NAME :: NAME() : pImpl(std::make_unique<Impl>()) {}

#endif //CONCERTO_MACRO_H
