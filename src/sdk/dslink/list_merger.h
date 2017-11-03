#ifndef DSA_SDK_LIST_MERGER_H
#define DSA_SDK_LIST_MERGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_set>
#include <vector>
#include "util/enable_ref.h"

namespace dsa {

class DsLink;
class ListMerger;
class ListResponseMessage;
class IncomingListStream;

class IncomingListCache : public DestroyableRef<IncomingListCache> {
  friend class ListMerger;

 public:
  typedef std::function<void(IncomingListCache&, const std::vector<string_>&)>
      Callback;

 protected:
  void destroy_impl() final;
  ref_<ListMerger> _merger;
  Callback _callback;

 public:
  IncomingListCache();
  IncomingListCache(ref_<ListMerger>&& merger,
                    IncomingListCache::Callback&& callback);
};
// when multiple list request is made on same path, ListMerger merge
// the subscription into one request
class ListMerger : public DestroyableRef<ListMerger> {
 protected:
  std::unordered_set<ref_<IncomingListCache>, RefHash<IncomingListCache> >
      caches;
  ref_<DsLink> _link;
  string_ _path;

  ref_<IncomingListStream> _stream;

  void destroy_impl() final;

  void new_list_response(ref_<const ListResponseMessage>&& message);

 public:
  explicit ListMerger(ref_<DsLink>&& link = nullptr, const string_& path = "");
  ~ListMerger();

  ref_<IncomingListCache> list(IncomingListCache::Callback&& callback);
  void remove(const ref_<IncomingListCache>& cache);
};
}

#endif  // DSA_SDK_LIST_MERGER_H
