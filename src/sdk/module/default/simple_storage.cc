#include "dsa_common.h"

#include "simple_storage.h"

namespace dsa {
std::unique_ptr<StorageBucket> SimpleStorage::get_bucket(const std::string& name) {
  if (name == "broker.json")
    return std::unique_ptr<StorageBucket>(new SimpleSafeStorageBucket(_io_service));
  else
    return std::unique_ptr<StorageBucket>(new SimpleStorageBucket(_io_service));
}

/// create a bucket or find a existing bucket
std::unique_ptr<QueueBucket> SimpleStorage::get_queue_bucket(const std::string& name) {
  return std::unique_ptr<QueueBucket>(new SimpleQueueBucket());
}

}  // namespace dsa
