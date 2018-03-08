#include "dsa_common.h"

#include "storage.h"

namespace dsa {

shared_ptr_<StorageBucket> Storage::get_strand_bucket(
    const string_ &name, boost::asio::io_service::strand *strand) {
  shared_ptr_<StorageBucket> storage_bucket = get_shared_bucket(name);

  storage_bucket->owner_strand = strand;

  return storage_bucket;
}
}