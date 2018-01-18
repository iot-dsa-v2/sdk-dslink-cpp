#ifndef DSA_SDK_SUBSCRIBE_MERGER_H
#define DSA_SDK_SUBSCRIBE_MERGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_set>
#include "message/message_options.h"
#include "util/enable_ref.h"

namespace dsa {

class DsLink;
class SubscribeMerger;
class SubscribeResponseMessage;
class IncomingSubscribeStream;

class IncomingSubscribeCache : public DestroyableRef<IncomingSubscribeCache> {
  friend class SubscribeMerger;

 public:
  typedef std::function<void(IncomingSubscribeCache&,
                             ref_<const SubscribeResponseMessage>&)>
      Callback;

 protected:
  void destroy_impl() final;
  ref_<SubscribeMerger> _merger;
  SubscribeOptions _options;
  Callback _callback;
  bool _callback_running = false;

  void _receive_update(ref_<const SubscribeResponseMessage>& message);

 public:
  IncomingSubscribeCache();
  IncomingSubscribeCache(ref_<SubscribeMerger>&& merger,
                         IncomingSubscribeCache::Callback&& callback,
                         const SubscribeOptions& options);

  void close() { destroy(); }
  bool is_closed() { return is_destroyed(); }
};
// when multiple subscribe request is made on same path, SubscribeMerger merge
// the subscription into one request
class SubscribeMerger : public DestroyableRef<SubscribeMerger> {
 protected:
  std::unordered_set<ref_<IncomingSubscribeCache>,
                     RefHash<IncomingSubscribeCache> >
      _caches;
  bool _iterating_caches = false;
  ref_<DsLink> _link;
  string_ _path;

  ref_<IncomingSubscribeStream> _stream;
  ref_<const SubscribeResponseMessage> _cached_value;

  void destroy_impl() final;

  void new_subscribe_response(ref_<const SubscribeResponseMessage>&& message);
  SubscribeOptions _merged_subscribe_options;
  void check_subscribe_options();

 public:
  explicit SubscribeMerger(ref_<DsLink>&& link = nullptr,
                           const string_& path = "");
  ~SubscribeMerger();

  ref_<IncomingSubscribeCache> subscribe(
      IncomingSubscribeCache::Callback&& callback,
      const SubscribeOptions& options);
  void remove(const ref_<IncomingSubscribeCache>& cache);
};
}

#endif  // DSA_SDK_SUBSCRIBE_MERGER_H
