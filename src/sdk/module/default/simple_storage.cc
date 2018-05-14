#include "dsa_common.h"

#include <boost/filesystem.hpp>
#include <codecvt>
#include <iomanip>
#include <iostream>
#include "simple_storage.h"
#include "util/string_encode.h"
namespace fs = boost::filesystem;

namespace dsa {

#if defined(_WIN32) || defined(_WIN64)

const string_ path_to_utf8_str(const boost::filesystem::path& path) {
  return std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(
      path.wstring());
}
fs::path utf8_str_to_path(const string_& str) {
  return fs::path(
      std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}
          .from_bytes(str));
}

#else

const string_ path_to_utf8_str(const boost::filesystem::path& path) {
  return path.string();
}
fs::path utf8_str_to_path(const string_& str) { return fs::path(str); }

#endif

shared_ptr_<SharedStorageBucket> SimpleStorage::get_shared_bucket(
    const std::string& name) {
  shared_ptr_<SharedStorageBucket> new_bucket;
  // for the special storage requirements, safe bucket can be returned
  if (name == "config")
    new_bucket = make_shared_<SimpleSafeStorageBucket>(name, _io_service, "");
  else
    new_bucket = make_shared_<SimpleStorageBucket>(name, _io_service);
  _bucket_list.push_back(new_bucket);
  return std::move(new_bucket);
}

void SimpleStorage::destroy_impl() {
  for (auto&& bucket : _bucket_list) {
    bucket->destroy_bucket();
    bucket.reset();
  }
  _bucket_list.clear();
}

void SimpleStorage::clear() {
  fs::path p(storage_default);
  if (fs::exists(p)) {
    for (auto&& x : fs::directory_iterator(p)) {
      shared_ptr_<StorageBucket> storage_bucket =
          get_shared_bucket(url_decode(path_to_utf8_str(x.path().filename())));
      storage_bucket->remove_all();
    }
  }
}

}  // namespace dsa
