#ifndef DSA_SDK_STRING_ENCODE_H
#define DSA_SDK_STRING_ENCODE_H

#if defined(_MSC_VER)
#pragma once
#endif

namespace dsa {

enum class StringEncodeLevel : uint8_t {
  URL_ENCODE_NODE_NAME = 2,
  URL_ENCODE_ASCII = 3,
  URL_ENCODE = 4,
};

// normal string encode/decode
string_ url_decode(const std::string& s_src);
string_ url_encode(const std::string& s_src,
                   StringEncodeLevel level = StringEncodeLevel::URL_ENCODE);

string_ url_encode_file_name(const std::string& s_src);

string_ url_encode_node_name(const std::string& s_src);
bool is_invalid_character(const char& c);

#if defined(_WIN32) || defined(_WIN64)
std::wstring string_to_wstring(const string_& s);
#endif

};  // namespace dsa

#endif  // DSA_SDK_STRING_ENCODE_H
