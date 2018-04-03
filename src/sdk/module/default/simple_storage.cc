#include "dsa_common.h"

#include <boost/filesystem.hpp>
#include <codecvt>
#include <iomanip>
#include <iostream>
#include "simple_storage.h"
#include "util/string_encode.h"
namespace fs = boost::filesystem;

namespace dsa {
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

/// create a bucket or find a existing bucket
std::unique_ptr<QueueBucket> SimpleStorage::get_queue_bucket(
    const string_& name) {
  return std::unique_ptr<QueueBucket>(new SimpleQueueBucket());
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
      std::wstring path_entry(x.path().stem().wstring());
      SimpleStorage simple_storage;
      shared_ptr_<StorageBucket> storage_bucket;
      storage_bucket = simple_storage.get_shared_bucket(url_decode(
          std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(
              path_entry)));
      storage_bucket->remove_all();
    }
  }
}

}  // namespace dsa
