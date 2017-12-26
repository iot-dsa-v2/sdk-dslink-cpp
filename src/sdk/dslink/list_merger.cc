#include "dsa_common.h"

#include "list_merger.h"

#include "core/client.h"
#include "link.h"
#include "message/response/list_response_message.h"
#include "stream/requester/incoming_list_stream.h"

namespace dsa {
IncomingListCache::IncomingListCache(){};
IncomingListCache::IncomingListCache(ref_<ListMerger>&& merger,
                                     IncomingListCache::Callback&& callback)
    : _merger(std::move(merger)), _callback(std::move(callback)) {}

void IncomingListCache::destroy_impl() {
  _merger->remove(get_ref());
  _merger.reset();
  _callback = nullptr;
}

const VarMap& IncomingListCache::get_map() const { return _merger->_map; }
MessageStatus IncomingListCache::get_status() const {
  return _merger->_last_status;
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

  for (auto it : caches) {
    it->destroy();
  }
  caches.clear();

  _changes.clear();
}

ref_<IncomingListCache> ListMerger::list(
    IncomingListCache::Callback&& callback) {
  IncomingListCache* cache =
      new IncomingListCache(get_ref(), std::move(callback));
  caches.emplace(cache);

  if (_stream == nullptr) {
    _stream = _link->_client->get_session().requester.list(_path, [
      this, copy_ref = get_ref()
    ](IncomingListStream & stream, ref_<const ListResponseMessage> && msg) {
      new_list_response(std::move(msg));
    });
  }
  if (_last_status != MessageStatus::INITIALIZING) {
    // send a fresh update
    cache->_callback(*cache, {});
  }
  return cache->get_ref();
}

void ListMerger::new_list_response(ref_<const ListResponseMessage>&& message) {
  bool refreshed = message->get_refreshed();
  _last_status = message->get_status();
  if (refreshed) {
    _map.clear();
    _changes.clear();
  }
  auto map = message->get_map();

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
  if (_last_status != MessageStatus::INITIALIZING) {
    for (auto& it : caches) {
      it->_callback(*it, _changes);
    }
    _changes.clear();
  }
}

void ListMerger::remove(const ref_<IncomingListCache>& cache) {
  if (is_destroyed()) return;

  caches.erase(cache);
  if (caches.empty()) {
    destroy();
  }
}
}
