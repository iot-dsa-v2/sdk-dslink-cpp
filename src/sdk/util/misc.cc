#include "dsa_common.h"
#include "misc.h"
#include <stdio.h>
#if (defined(_WIN32) || defined(_WIN64))
#include <direct.h>
#define GET_CURRENT_DIR _getcwd
#else
#include <unistd.h>
#define GET_CURRENT_DIR getcwd
#endif
#include <iostream>
//#include <boost/filesystem.hpp>

namespace dsa {

std::string url_encode(const std::string &s_src) {
  if (s_src.empty()) return "";
  const unsigned char dec_to_hex[16 + 1] = "0123456789ABCDEF";
  const unsigned char *p_src = (const unsigned char *)s_src.c_str();
  const auto src_len = s_src.length();
  unsigned char *const p_start = new unsigned char[src_len * 3];
  unsigned char *p_end = p_start;
  const unsigned char *const src_end = p_src + src_len;

  for (; p_src < src_end; ++p_src) {
    switch (*p_src) {
      case '%':
      case '=':
      case '&':
      case '\n':
        // escape this char
        *p_end++ = '%';
        *p_end++ = dec_to_hex[*p_src >> 4];
        *p_end++ = dec_to_hex[*p_src & 0x0F];
        break;
      default:
        *p_end++ = *p_src;
        break;
    }
  }

  std::string sResult((char *)p_start, (char *)p_end);
  delete[] p_start;
  return sResult;
}

string_ get_current_working_dir() {
  char buff[FILENAME_MAX];
  GET_CURRENT_DIR(buff, FILENAME_MAX);
  std::string current_working_dir(buff);
  return current_working_dir;
//  return boost::filesystem::current_path().string();
}
}