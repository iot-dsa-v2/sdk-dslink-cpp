#ifndef DSA_SDK_STRING_ENCODE_H
#define DSA_SDK_STRING_ENCODE_H

#if defined(_MSC_VER)
#pragma once
#endif

namespace dsa {

enum class StringEncodeLevel : uint8_t {
  URL_ENCODE_NODE_NAME = 2,
  URL_ENCODE_FILE_NAME = 2,
  URL_ENCODE = 4,
};

// normal string encode/decode
string_ url_decode(const std::string& s_src);
string_ url_encode(const std::string& s_src,
                   StringEncodeLevel level = StringEncodeLevel::URL_ENCODE);

string_ url_encode_file_name(const std::string& s_src);

string_ url_encode_node_name(const std::string& s_src);
bool is_invalid_character(const char& c);


// wide string encode/decode
// does not encode for >0x80
wstring_ url_encode_w(
    const wstring_& input,
    StringEncodeLevel level = StringEncodeLevel::URL_ENCODE);
wstring_ url_decode_w(const wstring_& input);
wstring_ url_encode_file_name_w(const wstring_& s_src);
wstring_ url_encode_node_name_w(const wstring_& s_src);

#if defined(_WIN32) || defined(_WIN64)
std::string url_encode(const std::wstring& input,
                       StringEncodeLevel level = StringEncodeLevel::URL_ENCODE);

std::wstring url_decode_w(const std::string& input);

// can be used when level-3 encoding, encodes >0x80
string_ url_encode_file_name(const std::wstring& s_src);
string_ url_encode_node_name(const std::wstring& s_src);

bool is_invalid_character_w(const wchar_t& c);

#endif

wstring_ string_to_wstring(const string_ &s);
string_ wstring_to_string(const wstring_ &s);
bool wstring_equals(const wstring_ &s1, const string_ &s2);

};  // namespace dsa

#endif  // DSA_SDK_STRING_ENCODE_H
