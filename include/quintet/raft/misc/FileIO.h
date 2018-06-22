#ifndef QUINTET_FILEIO_H
#define QUINTET_FILEIO_H

#include <string>
#include <map>
#include "Serialization.h"
#include <fstream>

namespace quintet {

class File {
public:
  typedef int file_type;
  static const int FILE_VALUE = 0;
  static const int FILE_VECTOR = 1;

  template<class T>
  void registerVar(const std::string &name, file_type type) {
    if (vars.find(name) != vars.end())
      throw std::runtime_error("\"" + name + "\" has already registered");
    else
      vars[name] = type;
  }

  template<class T>
  void modify(const std::string &name, const T &val) {
    checkVar(name, FILE_VALUE);
    std::string val_data = serialize<T>(val);
    std::string file_name = name + ".bin";
    std::fstream file(file_name, std::ios::binary | std::ios::out);
    if (!file)
      throw std::runtime_error("cannot open file");
    file.write(val_data.c_str(), val_data.length());
    file.close();
  };

  template<class T>
  T getValue(const std::string &name) {
    using namespace boost::hana::literals;
    checkVar(name, FILE_VALUE);
    std::string file_name = name + ".bin";
    std::fstream file(file_name, std::ios::binary | std::ios::in);
    if (!file)
      throw std::runtime_error("cannot open file");
    std::filebuf *pbuf = file.rdbuf();
    long long size = pbuf->pubseekoff(0, std::ios::end, std::ios::in);
    char *buffer = new char[size];
    pbuf->pubseekpos(0, std::ios::in);
    pbuf->sgetn(buffer, size);
    file.close();
    T value = deserialize<T>(buffer)[0_c];
    delete[] buffer;
    return value;
  }

  template<class T>
  void append(const std::string &name, const T &val) {
    checkVar(name, FILE_VECTOR);
    std::string file_name = name + ".bin";
    std::fstream file;
    int length = 4;
    file.open(file_name, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) {
      file.open(file_name, std::ios::binary | std::ios::out);
      file.write((char *) &length, 4);
    } else {
      file.read((char *) &length, 4);
    }
    file.seekp(length, file.beg);
    std::string val_data = serialize<T>(val);
    std::string data_size = int2String(val_data.length());
    val_data = data_size + val_data + data_size;
    file.write(val_data.c_str(), val_data.length());
    length += val_data.length();
    file.seekp(0, file.beg);
    file.write((char *) &length, 4);
    file.close();
  }

  template<class T>
  std::vector<T> getVector(const std::string &name) {
    using namespace boost::hana::literals;
    std::vector<T> ans;
    checkVar(name, FILE_VECTOR);
    std::string file_name = name + ".bin";
    std::fstream file;
    file.open(file_name, std::ios::binary | std::ios::in);
    if (!file)
      throw std::runtime_error("cannot open file");
    int length;
    file.read((char *) &length, 4);
    while (file.tellg() < length) {
      int size, size1;
      file.read((char *) &size, 4);
      char *buffer = new char[size];
      file.read(buffer, size);
      file.read((char *) &size1, 4);
      if (size != size1)
        std::runtime_error("file error");
      ans.push_back(deserialize<T>(buffer)[0_c]);
      delete[] buffer;
    }
    file.close();
    return ans;
  }

  template<class T>
  void pop(const std::string &name) {
    using namespace boost::hana::literals;
    std::vector<T> ans;
    checkVar(name, FILE_VECTOR);
    std::string file_name = name + ".bin";
    std::fstream file;
    file.open(file_name, std::ios::binary | std::ios::in | std::ios::out);
    if (!file)
      throw std::runtime_error("cannot open file");
    int length;
    file.read((char *) &length, 4);
    int size, size1;
    file.seekg(length - 4, file.beg);
    file.read((char *) &size, 4);
    file.seekg(-size - 8, file.cur);
    file.read((char *) &size1, 4);
    if (size != size1)
      std::runtime_error("file error");
    length -= size + 8;
    file.seekp(0, file.beg);
    file.write((char*) & length, 4);
    file.close();
  }

  template <class T>
  void clearFile(const std::string &name) {
    std::string file_name = name + ".bin";
    remove(file_name.c_str());
  }

private:
  std::map<std::string, int> vars;

  void checkVar(const std::string &name, file_type type);

  std::string int2String(int x);
}; // class File


} // namespace quintet

#endif //QUINTET_FILEIO_H
