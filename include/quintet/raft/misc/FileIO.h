#ifndef QUINTET_FILEIO_H
#define QUINTET_FILEIO_H

#include <string>

namespace quintet {

class File {
public:
  template <class T>
  void registerVar(const std::string & name);

  template <class T>
  void modify(const std::string & name, const T & val);

  template <class Container>
  void append(const std::string & name, const Container::value_type & val);
}; // class File

} // namespace quintet

#endif //QUINTET_FILEIO_H
