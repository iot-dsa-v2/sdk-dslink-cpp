//
// Created by mk on 12/14/17.
//

#include "string.h"
#include <boost/filesystem.hpp>

using namespace std;
namespace fs = boost::filesystem;

namespace dsa{

string_ string_from_file(string_ file_path){
  string_ data;
  if (fs::is_regular_file(file_path)) {
    std::ifstream my_file(file_path, std::ios::in | std::ios::binary);
    if (my_file.is_open()) {
      my_file >> data;
    } else {
      throw std::runtime_error("cannot open file to read");
    }
  }
  else{
    throw std::runtime_error("there is no file");
  }

  return data;
}

void string_to_file(string_ data, string_ file_path){
  std::ofstream my_file(file_path,
                        std::ios::out | std::ios::binary | std::ios::trunc);
  if (my_file.is_open()) {
    my_file<<data;
  } else {
    throw std::runtime_error("Unable to open file to write");
  }
}

}