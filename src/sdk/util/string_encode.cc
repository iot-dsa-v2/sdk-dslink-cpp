#include "dsa_common.h"
#include "string_encode.h"
namespace dsa {

static char HEX2DEC[256] =
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

//4: Only alphanum is safe
//3: if (c < ' ' || (c >= ':' && c <= '?')  || c == '\\' ||
// c == '\'' || c == '\"' || c == '/' || c == '*' || c == '|' || c == '%')
//2: 0x80 - 0xFF
//1: '%' sign
static char SAFE[256] =
    {
        /*      0 1 2 3  4 5 6 7  8 9 A B  C D E F */
        /* 0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* 1 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* 2 */ 3,0,3,3, 3,1,3,0, 3,3,0,3, 3,3,3,0,
        /* 3 */ 4,4,4,4, 4,4,4,4, 4,4,0,0, 0,0,0,0,

        /* 4 */ 3,4,4,4, 4,4,4,4, 4,4,4,4, 4,4,4,4,
        /* 5 */ 4,4,4,4, 4,4,4,4, 4,4,4,3, 0,3,3,3,
        /* 6 */ 0,4,4,4, 4,4,4,4, 4,4,4,4, 4,4,4,4,
        /* 7 */ 4,4,4,4, 4,4,4,4, 4,4,4,3, 3,3,3,3,

        /* 8 */ 2,2,2,2, 2,2,2,2, 2,2,2,2, 2,2,2,2,
        /* 9 */ 2,2,2,2, 2,2,2,2, 2,2,2,2, 2,2,2,2,
        /* A */ 2,2,2,2, 2,2,2,2, 2,2,2,2, 2,2,2,2,
        /* B */ 2,2,2,2, 2,2,2,2, 2,2,2,2, 2,2,2,2,

        /* C */ 2,2,2,2, 2,2,2,2, 2,2,2,2, 2,2,2,2,
        /* D */ 2,2,2,2, 2,2,2,2, 2,2,2,2, 2,2,2,2,
        /* E */ 2,2,2,2, 2,2,2,2, 2,2,2,2, 2,2,2,2,
        /* F */ 2,2,2,2, 2,2,2,2, 2,2,2,2, 2,2,2,2
    };

std::string url_encode(const std::string &s_src, StringEncodeLevel level) {
  if (s_src.empty()) return "";
  const unsigned char dec_to_hex[16 + 1] = "0123456789ABCDEF";
  const unsigned char *p_src = (const unsigned char *)s_src.c_str();
  const auto src_len = s_src.length();
  unsigned char *const p_start = new unsigned char[src_len * 3];
  unsigned char *p_end = p_start;
  const unsigned char *const src_end = p_src + src_len;

  for (; p_src < src_end; ++p_src) {
    if (SAFE[*p_src] >= static_cast<char>(level))
      *p_end++ = *p_src;
    else
    {
      // escape this char
      *p_end++ = '%';
      *p_end++ = dec_to_hex[*p_src >> 4];
      *p_end++ = dec_to_hex[*p_src & 0x0F];
    }
  }

  std::string s_result((char *)p_start, (char *)p_end);
  delete[] p_start;
  return s_result;
}
string_ url_encode_file_name(const string_ &s_src) {
  return url_encode(s_src, StringEncodeLevel::URL_ENCODE_FILE_NAME);
}
string_ url_encode_node_name(const string_ &s_src) {
  return url_encode(s_src, StringEncodeLevel::URL_ENCODE_NODE_NAME);
}
bool is_invalid_character(const char& c) {
  return SAFE[c] > 0;
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

}