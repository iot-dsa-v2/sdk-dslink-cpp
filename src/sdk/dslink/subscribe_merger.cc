#include "dsa_common.h"

#include "subscribe_merger.h"

namespace dsa {
IncommingSubscribeCache::IncommingSubscribeCache(ref_<SubscribeMerger>&& merger)
    : _merger(std::move(merger)) {}
void IncommingSubscribeCache::destroy_impl() { _merger->remove(get_ref()); }

void SubscribeMerger::remove(const ref_<IncommingSubscribeCache>& cache) {
  caches.erase(cache);
  if (caches.empty()) {

  }
}
}