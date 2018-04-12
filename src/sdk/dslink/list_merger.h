#ifndef DSA_SDK_LIST_MERGER_H
#define DSA_SDK_LIST_MERGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_set>
#include <vector>
#include "util/enable_ref.h"
#include "util/enums.h"
#include "variant/variant.h"

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
  bool _callback_running = false;
  bool _list_profile = false;

  void _receive_update(const std::vector<string_>& update);

 public:
  IncomingListCache();
  IncomingListCache(ref_<ListMerger>&& merger,
                    IncomingListCache::Callback&& callback, bool list_profile);
  const VarMap& get_map() const;
  const VarMap& get_profile_map() const;
  Status get_status() const;
  const string_& get_last_pub_path() const;

  void close() { destroy(); }
  bool is_closed() { return is_destroyed(); }
};
// when multiple list request is made on same path, ListMerger merge
// the subscription into one request
class ListMerger : public DestroyableRef<ListMerger> {
  friend class IncomingListCache;

 protected:
  ref_<IncomingListCache> _profile_list_cache;

  std::unordered_set<ref_<IncomingListCache>, RefHash<IncomingListCache> >
      _caches;
  bool _iterating_caches = false;
  ref_<DsLink> _link;
  string_ _path;

  ref_<IncomingListStream> _stream;
  VarMap _map;
  ref_<VarMap> _profile_map;
  std::vector<string_> _changes;
  Status _last_status = Status::INITIALIZING;
  string_ _last_pub_path;
  bool _profile_required = false;

  void destroy_impl() final;

  void new_list_response(ref_<const ListResponseMessage>&& message);
  void update_caches(std::vector<string_> changes, bool profile_only = false);

 public:
  explicit ListMerger(ref_<DsLink>&& link = nullptr, const string_& path = "");
  ~ListMerger();

  ref_<IncomingListCache> list(IncomingListCache::Callback&& callback,
                               bool list_profile);
  void list_profile();
  void remove(const ref_<IncomingListCache>& cache);
};
}  // namespace dsa

#endif  // DSA_SDK_LIST_MERGER_H
