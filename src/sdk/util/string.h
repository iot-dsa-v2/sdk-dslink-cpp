#ifndef DSA_UTIL_STRING_H
#define DSA_UTIL_STRING_H

#if defined(_MSC_VER)
#pragma once
#endif

namespace dsa {
inline bool str_starts_with(const string_ &str,
                            const string_ &pattern) {
  return str.size() >= pattern.size() &&
         std::equal(pattern.begin(), pattern.end(), str.begin());
}

inline string_ str_join_path(const string_ &path1,
                             const string_ &path2) {
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

string_ string_from_file(string_ file_path);
void string_to_file(string_ data, string_ file_path);


static int IS_RAND_INITIALIZED = 0;
string_ generate_random_string(int len);

// todo, handle token loading from StorageBucket and remove this function
string_ get_close_token_from_file(string_ path_str=".close_token", bool force_to_generate_one = false);
}

#endif  // DSA_UTIL_STRING_H
