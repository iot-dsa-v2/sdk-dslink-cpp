#ifndef DSA_UTIL_STRING_H
#define DSA_UTIL_STRING_H

#if defined(_MSC_VER)
#pragma once
#endif

namespace dsa {
inline bool str_starts_with(const std::string &str,
                            const std::string &pattern) {
  return str.size() >= pattern.size() &&
         std::equal(pattern.begin(), pattern.end(), str.begin());
}

inline string_ str_join_path(const std::string &path1,
                             const std::string &path2) {
  if (path1.empty()) return path2;
  if (path2.empty()) return path1;
  static const string_ slash = "/";
  string_ result;
  result.reserve(path1.length() + path2.length() + 1);
  result.append(path1);
  result.append(slash);
  result.append(path2);
  return std::move(result);
}
}

#endif  // DSA_UTIL_STRING_H
