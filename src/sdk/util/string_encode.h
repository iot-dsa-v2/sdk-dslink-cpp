#ifndef DSA_SDK_STRING_ENCODE_H
#define DSA_SDK_STRING_ENCODE_H

#if defined(_MSC_VER)
#pragma once
#endif

namespace dsa {
string_ url_decode(const std::string & s_src);
string_ url_encode(const std::string& s_src, const char *safe_list = nullptr);
string_ url_encode_path_name(const std::string& s_src);
};

#endif //DSA_SDK_STRING_ENCODE_H
