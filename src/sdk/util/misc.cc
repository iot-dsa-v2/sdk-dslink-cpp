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

const char HEX2DEC[256] =
{
    /*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
    /* 0 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 1 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 2 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 3 */  0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,

    /* 4 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 5 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 6 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 7 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

    /* 8 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 9 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* A */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* B */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

    /* C */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* D */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* E */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* F */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
};
// Only alphanum is safe.
const char SAFE[256] =
    {
        /*      0 1 2 3  4 5 6 7  8 9 A B  C D E F */
        /* 0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* 1 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* 2 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* 3 */ 1,1,1,1, 1,1,1,1, 1,1,0,0, 0,0,0,0,

        /* 4 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
        /* 5 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
        /* 6 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
        /* 7 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,

        /* 8 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* 9 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* A */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* B */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,

        /* C */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* D */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* E */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* F */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
    };


std::string url_encode(const std::string &s_src) {
  if (s_src.empty()) return "";
  const unsigned char dec_to_hex[16 + 1] = "0123456789ABCDEF";
  const unsigned char *p_src = (const unsigned char *)s_src.c_str();
  const auto src_len = s_src.length();
  unsigned char *const p_start = new unsigned char[src_len * 3];
  unsigned char *p_end = p_start;
  const unsigned char *const src_end = p_src + src_len;

  for (; p_src < src_end; ++p_src) {
    if (SAFE[*p_src])
      *p_end++ = *p_src;
    else
    {
      // escape this char
      *p_end++ = '%';
      *p_end++ = dec_to_hex[*p_src >> 4];
      *p_end++ = dec_to_hex[*p_src & 0x0F];
    }

  }

  std::string sResult((char *)p_start, (char *)p_end);
  delete[] p_start;
  return sResult;
}

std::string url_decode(const std::string & s_src)
{
  // Note from RFC1630: "Sequences which start with a percent
  // sign but are not followed by two hexadecimal characters
  // (0-9, A-F) are reserved for future extension"

  const unsigned char * p_src = (const unsigned char *)s_src.c_str();
  const int src_len = s_src.length();
  const unsigned char * const src_end = p_src + src_len;
  // last decodable '%'
  const unsigned char * const SRC_LAST_DEC = src_end - 2;

  char * const p_start = new char[src_len];
  char * p_end = p_start;

  while (p_src < SRC_LAST_DEC)
  {
    if (*p_src == '%')
    {
      char dec1, dec2;
      if (-1 != (dec1 = HEX2DEC[*(p_src + 1)])
          && -1 != (dec2 = HEX2DEC[*(p_src + 2)]))
      {
        *p_end++ = (dec1 << 4) + dec2;
        p_src += 3;
        continue;
      }
    }

    *p_end++ = *p_src++;
  }

  // the last 2- chars
  while (p_src < src_end)
    *p_end++ = *p_src++;

  std::string sResult(p_start, p_end);
  delete [] p_start;
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