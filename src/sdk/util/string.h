#ifndef DSA_UTIL_STRING_H
#define DSA_UTIL_STRING_H

#if defined(_MSC_VER)
#pragma once
#endif

namespace dsa {
class StorageBucket;
inline bool str_starts_with(const string_ &str, const string_ &pattern) {
  return str.size() >= pattern.size() &&
         std::equal(pattern.begin(), pattern.end(), str.begin());
}

inline string_ str_join_path(const string_ &path1, const string_ &path2) {
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

string_ string_from_file(const wstring_ &file_path);
void string_to_file(const string_ &data, const wstring_ &file_path);

string_ string_from_storage(const wstring_ &key, StorageBucket &storage_bucket);
void string_to_storage(const string_ &data, const wstring_ &key, StorageBucket &storage_bucket);

static int IS_RAND_INITIALIZED = 0;
string_ generate_random_string(int len);
#if defined(_WIN32) || defined(_WIN64)
static constexpr wchar_t default_close_token_path[] = L".close_token";
# else 
static constexpr char default_close_token_path[] = ".close_token";
# endif
// todo, handle token loading from StorageBucket and remove this function
string_ get_close_token_from_storage(StorageBucket &storage_bucket,
                                  const wstring_ &key= default_close_token_path,
                                  bool force_to_generate_one = false);
}

#endif  // DSA_UTIL_STRING_H
