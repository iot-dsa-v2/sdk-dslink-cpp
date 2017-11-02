#ifndef DSA_SDK_SUBSCRIBE_MERGER_H
#define DSA_SDK_SUBSCRIBE_MERGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_set>
#include "util/enable_ref.h"

namespace dsa {

class SubscribeMerger;

class IncommingSubscribeCache : public DestroyableRef<IncommingSubscribeCache> {
 protected:
  void destroy_impl() final;
  ref_<SubscribeMerger> _merger;

 public:
  explicit IncommingSubscribeCache(ref_<SubscribeMerger> && merger);
};
// when multiple subscribe request is made on same path, SubscribeMerger merge
// the subscription into one request
class SubscribeMerger : public EnableRef<SubscribeMerger> {
  struct Hash {
    size_t operator()(const ref_<IncommingSubscribeCache>& ref) const {
      return reinterpret_cast<size_t>(ref.get());
    }
  };

  std::unordered_set<ref_<IncommingSubscribeCache>, Hash> caches;

public:
  void remove(const ref_<IncommingSubscribeCache> & cache);
};
}

#endif  // DSA_SDK_SUBSCRIBE_MERGER_H
