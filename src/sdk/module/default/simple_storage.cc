#include "dsa_common.h"

#include "simple_storage.h"
#include "util/string_encode.h"

namespace dsa {
std::unique_ptr<StorageBucket> SimpleStorage::get_bucket(
    const wstring_& name) {
  //for the special storage requirements, safe bucket can be returned
  if (wstring_equals(name, "config"))
    return std::unique_ptr<StorageBucket>(
        new SimpleSafeStorageBucket(name, _io_service, empty_wstring));
  else
    return std::unique_ptr<StorageBucket>(
        new SimpleStorageBucket(name, _io_service));
}

/// create a bucket or find a existing bucket
std::unique_ptr<QueueBucket> SimpleStorage::get_queue_bucket(
    const wstring_& name) {
  return std::unique_ptr<QueueBucket>(new SimpleQueueBucket());
}

}  // namespace dsa
