#ifndef DSA_SDK_MISC_H
#define DSA_SDK_MISC_H

#if defined(_MSC_VER)
#pragma once
#endif

namespace dsa {
string_ url_decode(const std::string & s_src);
string_ url_encode(const std::string& s_src);
string_ get_current_working_dir();
};

#endif  // DSA_SDK_MISC_H
