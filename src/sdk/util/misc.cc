#include "dsa_common.h"
#include "misc.h"
#if (defined(_WIN32) || defined(_WIN64))
#include <direct.h>
#define GET_CURRENT_DIR _getcwd
#else
#include <unistd.h>
#define GET_CURRENT_DIR getcwd
#endif
#include <iostream>
#include <boost/filesystem.hpp>

namespace dsa {
string_ get_current_working_dir() {
  char buff[FILENAME_MAX];
  GET_CURRENT_DIR(buff, FILENAME_MAX);
  std::string current_working_dir(buff);
  return current_working_dir;
//  return boost::filesystem::current_path().string();
}

wstring_ get_current_working_dir_w() {
#if (defined(_WIN32) || defined(_WIN64))
  return boost::filesystem::current_path().wstring();
#else
  return boost::filesystem::current_path().string();
#endif
}
}