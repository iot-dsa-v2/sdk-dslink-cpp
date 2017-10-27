#include "simple_storage.h"

namespace dsa {
StorageBucket& SimpleStorage::get_bucket(const std::string& name) {
  return *(new SimpleStorageBucket());
}

/// create a bucket or find a existing bucket
QueueBucket& SimpleStorage::get_queue_bucket(const std::string& name) {
  return *(new SimpleQueueBucket());
}

}  // namespace dsa

