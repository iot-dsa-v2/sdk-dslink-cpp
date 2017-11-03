#ifndef DSA_SDK_LIST_MERGER_H
#define DSA_SDK_LIST_MERGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_set>
#include <vector>
#include "util/enable_ref.h"
#include "variant/variant.h"
#include "message/enums.h"

namespace dsa {

class DsLink;
class ListMerger;
class ListResponseMessage;
class IncomingListStream;

class IncomingListCache : public DestroyableRef<IncomingListCache> {
  friend class ListMerger;

 public:
  // a callback function that has all the changed keys
  // when the vector is empty, that means all value are changed
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
  const VarMap& get_map() const;
  MessageStatus get_status() const;
};
// when multiple list request is made on same path, ListMerger merge
// the subscription into one request
class ListMerger : public DestroyableRef<ListMerger> {
  friend class IncomingListCache;

 protected:
  std::unordered_set<ref_<IncomingListCache>, RefHash<IncomingListCache> >
      caches;
  ref_<DsLink> _link;
  string_ _path;

  ref_<IncomingListStream> _stream;
  VarMap _map;
  std::vector<string_> _changes;
  MessageStatus _last_status = MessageStatus::INITIALIZING;

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
