#include "dsa_common.h"

#include "list_merger.h"

#include "core/client.h"
#include "link.h"
#include "message/response/list_response_message.h"
#include "module/logger.h"
#include "stream/requester/incoming_list_stream.h"

namespace dsa {
IncomingListCache::IncomingListCache(){};
IncomingListCache::IncomingListCache(ref_<ListMerger>&& merger,
                                     IncomingListCache::Callback&& callback,
                                     bool list_profile)
    : _merger(std::move(merger)),
      _callback(std::move(callback)),
      _list_profile(list_profile) {}

void IncomingListCache::destroy_impl() {
  _merger->remove(get_ref());
  _merger.reset();
  if (!_callback_running) {
    _callback = nullptr;
  }
}

void IncomingListCache::_receive_update(const std::vector<string_>& update) {
  if (_callback != nullptr) {
    BEFORE_CALLBACK_RUN();
    _callback(*this, update);
    AFTER_CALLBACK_RUN();
  }
}

const VarMap& IncomingListCache::get_map() const { return _merger->_map; }
const VarMap& IncomingListCache::get_profile_map() const {
  return *_merger->_profile_map;
}
Status IncomingListCache::get_status() const { return _merger->_last_status; }
const string_& IncomingListCache::get_last_pub_path() const {
  return _merger->_last_pub_path;
}

ListMerger::ListMerger(ref_<DsLink>&& link, const string_& path)
    : _link(std::move(link)), _path(path) {}
ListMerger::~ListMerger() {}

void ListMerger::destroy_impl() {
  _link->_list_mergers.erase(_path);
  _link.reset();

  if (_stream != nullptr) {
    _stream->close();
    // stream can't be destroyed here to make sure the request is canceled
    _stream.reset();
  }
  if (_profile_list_cache != nullptr) {
    _profile_list_cache->close();
    _profile_list_cache.reset();
  }

  for (auto it : _caches) {
    it->close();
  }
  _caches.clear();

  _changes.clear();
}

ref_<IncomingListCache> ListMerger::list(IncomingListCache::Callback&& callback,
                                         bool list_profile) {
  if (list_profile) {
    _profile_required = true;
  }
  IncomingListCache* cache =
      new IncomingListCache(get_ref(), std::move(callback), list_profile);
  _caches.emplace(cache);

  if (_stream == nullptr) {
    _stream = _link->_client->get_session().requester.list(
        _path,
        CAST_LAMBDA(IncomingListStreamCallback)[this, copy_ref = get_ref()](
            IncomingListStream & stream,
            ref_<const ListResponseMessage> && msg) {
          if (msg) {
            new_list_response(std::move(msg));
          }
        });
  } else if (_last_status != Status::INITIALIZING) {
    // send a fresh update
    if (!list_profile || _profile_list_cache == nullptr ||
        _profile_map != nullptr) {
      // if profile is needed, check if
      cache->_receive_update({});
    }
  }
  return cache->get_ref();
}
void ListMerger::list_profile() {
  if (_profile_list_cache != nullptr) {
    _profile_list_cache->close();
  }
  // if $is and pub_path both exists
  auto is_str = _map.at("$is").to_string();
  _profile_list_cache = _link->list(
      _last_pub_path + "/" + is_str,
      [ this, keepref = get_ref() ](IncomingListCache & cache,
                                    const std::vector<string_>& str) mutable {
        _profile_map = make_ref_<VarMap>(cache.get_map());
        if (_last_status != Status::INITIALIZING) {
          // profile only udpate
          update_caches({}, true);
        }
      },
      false);
}
void ListMerger::update_caches(std::vector<string_> changes,
                               bool profile_only) {
  _iterating_caches = true;
  bool removed_some = false;
  for (auto it = _caches.begin(); it != _caches.end();) {
    if ((*it)->_list_profile) {
      if (_profile_list_cache != nullptr && _profile_map == nullptr) {
        // profile is reuqired, and it's availible, but not loaded yet
        ++it;
        continue;
      }
    } else if (profile_only) {
      // profile only updates is not need for list cache that doesn't require it
      ++it;
      continue;
    }
    (*it)->_receive_update(changes);
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
    }
  }
  _iterating_caches = false;
  _changes.clear();
}
void ListMerger::new_list_response(ref_<const ListResponseMessage>&& message) {
  bool refreshed = message->get_refreshed();
  _last_status = message->get_status();
  _last_pub_path = message->get_pub_path();

  if (refreshed) {
    _map.clear();
    _changes.clear();
  }
  auto& map = message->get_map();

  for (auto& it : map) {
    if (it.second->is_blank()) {
      _map.erase(it.first);
    } else {
      _map[it.first] = it.second->get_value();
    }
    if (!refreshed) {
      if (std::find(_changes.begin(), _changes.end(), it.first) ==
          _changes.end()) {
        _changes.push_back(it.first);
      }
    }
  }
  if (_profile_required && _profile_list_cache == nullptr &&
      _map.count("$is") > 0 && !_last_pub_path.empty()) {
    list_profile();
  }
  if (_last_status != Status::INITIALIZING) {
    // normal update
    update_caches(_changes);
  }
}

void ListMerger::remove(const ref_<IncomingListCache>& cache) {
  if (is_destroyed() || _iterating_caches) return;

  _caches.erase(cache);
  if (_caches.empty()) {
    destroy();
  }
}
}  // namespace dsa
