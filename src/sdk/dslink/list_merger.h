#ifndef DSA_SDK_LIST_MEGER_H
#define DSA_SDK_LIST_MEGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_set>
#include "util/enable_ref.h"

namespace dsa {

class ListMerger;

class IncommingListCache : public DestroyableRef<IncommingListCache> {

};

// when multiple list request is made on same path, ListMerger merge
// the subscription into one request
class ListMerger : public EnableRef<ListMerger> {
  struct Hash {
    size_t operator()(const ref_<IncommingListCache> &ref) const {
      return reinterpret_cast<size_t>(ref.get());
    }
  };
  std::unordered_set<ref_<IncommingListCache>, Hash> caches;
};
}

#endif  // DSA_SDK_LIST_MEGER_H
