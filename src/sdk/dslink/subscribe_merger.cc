#include "dsa_common.h"

#include "subscribe_merger.h"

#include "core/client.h"
#include "link.h"
#include "module/logger.h"
#include "stream/requester/incoming_subscribe_stream.h"
#include "message/response/subscribe_response_message.h"

namespace dsa {
IncomingSubscribeCache::IncomingSubscribeCache(){};
IncomingSubscribeCache::IncomingSubscribeCache(
    ref_<SubscribeMerger>&& merger, IncomingSubscribeCache::Callback&& callback,
    const SubscribeOptions& options)
    : _merger(std::move(merger)),
      _callback(std::move(callback)),
      _options(options) {}

void IncomingSubscribeCache::destroy_impl() {
  _merger->remove(get_ref());
  _merger.reset();
  if (!_callback_running) {
    _callback = nullptr;
  }
}

void IncomingSubscribeCache::_receive_update(
    ref_<const SubscribeResponseMessage>& message) {
  if (_callback != nullptr) {
    BEFORE_CALLBACK_RUN();
    _callback(*this, message);
    AFTER_CALLBACK_RUN();
  }
}

SubscribeMerger::SubscribeMerger(ref_<DsLink>&& link, const string_& path)
    : _link(std::move(link)),
      _path(path),
      _merged_subscribe_options(QosLevel::_0, -1) {}

SubscribeMerger::~SubscribeMerger() {}

void SubscribeMerger::destroy_impl() {
  _link->_subscribe_mergers.erase(_path);
  _link.reset();
  _cached_value.reset();

  for (auto it : _caches) {
    it->destroy();
  }
  _caches.clear();

  if (_stream != nullptr) {
    _stream->close();
    // stream can't be destroyed here to make sure the request is canceled
    _stream.reset();
  }
}

ref_<IncomingSubscribeCache> SubscribeMerger::subscribe(
    IncomingSubscribeCache::Callback&& callback,
    const SubscribeOptions& options) {
  auto cache = make_ref_<IncomingSubscribeCache>(get_ref(), std::move(callback),
                                                 options);

  _caches.emplace(cache);

  if (_stream == nullptr) {
    _merged_subscribe_options = options;
    _stream = _link->_client->get_session().subscribe(
        _path, [ this, copy_ref = get_ref() ](
                   IncomingSubscribeStream & stream,
                   ref_<const SubscribeResponseMessage> && msg) {
          new_subscribe_response(std::move(msg));
        },
        _merged_subscribe_options);
  } else if (_merged_subscribe_options.mergeFrom(cache->_options)) {
    _stream->subscribe(options);
  }
  if (_cached_value != nullptr) {
    cache->_receive_update(_cached_value);
  }

  return std::move(cache);
}

void SubscribeMerger::new_subscribe_response(
    SubscribeResponseMessageCRef&& message) {
  _cached_value = std::move(message);
  _iterating_caches = true;
  bool removed_some = false;
  for (auto it = _caches.begin(); it != _caches.end();) {
    (*it)->_receive_update(_cached_value);
    if ((*it)->is_destroyed()) {
      // remove() is blocked, need to handle it here
      it = _caches.erase(it);
      removed_some = true;
    } else {
      ++it;
    }
  }
  if (removed_some) {
    if (_caches.empty()) {
      destroy();
    } else {
      check_subscribe_options();
    }
  }
  _iterating_caches = false;
}

void SubscribeMerger::check_subscribe_options() {
  SubscribeOptions new_options(QosLevel::_0, -1);
  for (auto& cache : _caches) {
    new_options.mergeFrom(cache->_options);
  }
  if (new_options != _merged_subscribe_options) {
    _merged_subscribe_options = new_options;
    _stream->subscribe(_merged_subscribe_options);
  }
}

void SubscribeMerger::remove(const ref_<IncomingSubscribeCache>& cache) {
  if (is_destroyed() || _iterating_caches) return;
  _caches.erase(cache);
  if (_caches.empty()) {
    destroy();
  } else if (_merged_subscribe_options.needUpdateOnRemoval(cache->_options)) {
    check_subscribe_options();
  }
}
}
