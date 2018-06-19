#ifndef QUINTET_MACRO_H
#define QUINTET_MACRO_H

#define GEN_COPY(NAME, TYPE)                                                   \
  NAME(const NAME &) = TYPE;                                                   \
  NAME &operator=(const NAME &) = TYPE;

#define GEN_MOVE(NAME, TYPE)                                                   \
  NAME(NAME &&) = TYPE;                                                        \
  NAME &operator=(NAME &&) = TYPE;

#define GEN_DEFAULT_CTOR_AND_ASSIGNMENT(NAME)                                  \
  NAME() = default;                                                            \
  GEN_COPY(NAME, default)                                                      \
  GEN_MOVE(NAME, default)

#define GEN_CONST_HANDLE(NAME)                                                 \
  const auto &get_##NAME() const { return NAME; }

#endif // QUINTET_MACRO_H
