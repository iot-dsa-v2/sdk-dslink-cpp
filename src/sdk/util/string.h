#ifndef DSA_UTIL_STRING_H
#define DSA_UTIL_STRING_H

#if defined(_MSC_VER)
#pragma once
#endif

namespace dsa {
bool str_starts_with(const std::string &str, const std::string &pattern) {
  return str.size() >= pattern.size() &&
         std::equal(pattern.begin(), pattern.end(), str.begin());
}
}

#endif  // DSA_UTIL_STRING_H
