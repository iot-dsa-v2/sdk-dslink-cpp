#include "dsa_common.h"

#include "simple_storage.h"
#include "util/string_encode.h"

namespace dsa {
shared_ptr_<StorageBucket> SimpleStorage::get_shared_bucket(
    const std::string& name) {
  shared_ptr_<StorageBucket> new_bucket;
  //for the special storage requirements, safe bucket can be returned
  if (name == "config")
    new_bucket = shared_ptr_<StorageBucket>(
        new SimpleSafeStorageBucket(name, _io_service, ""));
  else
    new_bucket = shared_ptr_<StorageBucket>(
        new SimpleStorageBucket(name, _io_service));
  _bucket_list.push_back(new_bucket);
  return new_bucket;
}

/// create a bucket or find a existing bucket
std::unique_ptr<QueueBucket> SimpleStorage::get_queue_bucket(
    const string_& name) {
  return std::unique_ptr<QueueBucket>(new SimpleQueueBucket());
}

void SimpleStorage::destroy_impl() {
  for(auto &&bucket : _bucket_list) {
    bucket->destroy();
    bucket.reset();
  }
  _bucket_list.clear();
}

}  // namespace dsa
