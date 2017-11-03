#include "dsa_common.h"

#include "list_merger.h"

#include "core/client.h"
#include "link.h"
#include "stream/requester/incoming_list_stream.h"

namespace dsa {
IncomingListCache::IncomingListCache(){};
IncomingListCache::IncomingListCache(ref_<ListMerger>&& merger,
                                     IncomingListCache::Callback&& callback)
    : _merger(std::move(merger)), _callback(std::move(callback)) {}
void IncomingListCache::destroy_impl() { _merger->remove(get_ref()); }

ListMerger::ListMerger(ref_<DsLink>&& link, const string_& path)
    : _link(std::move(link)), _path(path) {}
ListMerger::~ListMerger() {}

void ListMerger::destroy_impl() {
  if (_stream != nullptr) {
    _stream->close();
    _stream.reset();
  }
  _link->_list_mergers.erase(_path);
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
}

void ListMerger::new_list_response(ref_<const ListResponseMessage>&& message) {
  for (auto& it : caches) {
  }
}

void ListMerger::remove(const ref_<IncomingListCache>& cache) {
  caches.erase(cache);
  if (caches.empty()) {
    destroy();
  }
}
}
