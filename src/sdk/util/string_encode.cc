#include "dsa_common.h"
#include "string_encode.h"
#include <iostream>
#include <string>
#include <locale>
#include <codecvt>
#include <iomanip>
#include <vector>

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
    else {
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
std::string url_decode(const std::string &s_src) {
  // Note from RFC1630: "Sequences which start with a percent
  // sign but are not followed by two hexadecimal characters
  // (0-9, A-F) are reserved for future extension"

  const unsigned char *p_src = (const unsigned char *)s_src.c_str();
  const int src_len = s_src.length();
  const unsigned char *const src_end = p_src + src_len;
  // last decodable '%'
  const unsigned char *const SRC_LAST_DEC = src_end - 2;

  char *const p_start = new char[src_len];
  char *p_end = p_start;

  while (p_src < SRC_LAST_DEC) {
    if (*p_src == '%') {
      char dec1, dec2;
      if (-1 != (dec1 = HEX2DEC[*(p_src + 1)]) &&
          -1 != (dec2 = HEX2DEC[*(p_src + 2)])) {
        *p_end++ = (dec1 << 4) + dec2;
        p_src += 3;
        continue;
      }
    }

    *p_end++ = *p_src++;
  }

  // the last 2- chars
  while (p_src < src_end) *p_end++ = *p_src++;

  std::string sResult(p_start, p_end);
  delete[] p_start;
  return sResult;
}
bool is_invalid_character(const char &c) { return SAFE[c] > 0; }


#if defined(_WIN32) || defined(_WIN64)
std::wstring url_encode_file_name_w(const std::wstring& s_src) {
  return url_encode_w(s_src, StringEncodeLevel::URL_ENCODE_FILE_NAME);
}
string_ url_encode_file_name(const std::wstring &s_src) {
  return url_encode(s_src, StringEncodeLevel::URL_ENCODE_FILE_NAME);
}
string_ url_encode_node_name(const std::wstring &s_src) {
  return url_encode(s_src, StringEncodeLevel::URL_ENCODE_NODE_NAME);
}
std::wstring url_encode_node_name_w(const std::wstring& s_src) {
  return url_encode_w(s_src, StringEncodeLevel::URL_ENCODE_NODE_NAME);
}
std::string url_encode(const std::wstring &input, StringEncodeLevel level) {
  std::string output;
  for (int i = 0; i < input.size(); i++) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv1;
    std::string u8str = conv1.to_bytes(input.substr(i, 1).c_str());
    if (u8str.size() > 0) {
      if (u8str.size() == 1 && u8str[0] < 0x80 &&
          (SAFE[u8str[0]] >= static_cast<char>(level))) {
        output.append(1, (static_cast<char>(input[i])));
      } else {
        for (auto &u8_char : u8str) {
          char onehex[5];
          snprintf(onehex, sizeof(onehex), "%%%2.2X", (unsigned char)u8_char);
          output.append(onehex);
        }
      }
    }
  }
  return output;
}
std::wstring string_to_wstring(const std::string &s) {
  std::wstring temp(s.length(), L' ');
  std::copy(s.begin(), s.end(), temp.begin());
  return temp;
}
std::wstring url_encode_w(const std::wstring &input, StringEncodeLevel level) {
  std::wstring output;
  for (int i = 0; i < input.size(); i++) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv1;
    std::string u8str = conv1.to_bytes(input.substr(i, 1).c_str());
    if (u8str.size() > 0) {
      if (u8str.size() > 1 ||
          (u8str.size() == 1 && (u8str[0] < 0x80 &&
                                 SAFE[u8str[0]] >= static_cast<char>(level)) ||
           (u8str[0] >= 0x80))) {
        output.append(1, (static_cast<wchar_t>(input[i])));
      } else {
        for (auto &u8_char : u8str) {
          char onehex[5];
          snprintf(onehex, sizeof(onehex), "%%%2.2X", (unsigned char)u8_char);
          output.append(string_to_wstring(onehex));
        }
      }
    }
  }
  return output;
}

std::wstring unhexlify(const std::wstring &input) {
  std::wstring output;
  for (const wchar_t *p = input.c_str(); *p;) {
    if (p[0] == '%' && isxdigit(p[1]) && isxdigit(p[2])) {
      int ch = (isdigit(p[1]) ? p[1] - '0' : toupper(p[1]) - 'A' + 10) * 16 +
               (isdigit(p[2]) ? p[2] - '0' : toupper(p[2]) - 'A' + 10);
      output.push_back((char)ch);
      p += 3;
    } else {
      output.push_back(*p++);
    }
  }
  return output;
}

std::wstring url_decode_w(const std::string &input) {
  std::wstring output, out2;
  std::string utf8 = url_decode(input);
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> conv;
  std::wstring wstr = conv.from_bytes(utf8);

  return wstr;
}
std::wstring url_decode_w(const std::wstring &input) {
  std::wstring output, out2;
  std::wstring unhex = unhexlify(input);
  return unhex;
}
bool is_invalid_character_w(const wchar_t &c) {
  return c > 0xFF || (c <= 0xFF && SAFE[c] > 0);
}
#endif
}