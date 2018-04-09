#include "dsa_common.h"

#include "storage.h"

#include "module/default/simple_storage.h"
#include "module/logger.h"
#include "util/string_encode.h"

namespace dsa {

StorageBucket& Storage::get_config_bucket() {
  static shared_ptr_<SimpleSafeStorageBucket> config_bucket =
      make_shared_<SimpleSafeStorageBucket>("config", nullptr, "");
  return *config_bucket;
}

ref_<StrandStorageBucket> Storage::get_strand_bucket(
    const string_& name, const LinkStrandRef& strand) {
  ref_<StrandStorageBucket> storage_bucket =
      make_ref_<StrandStorageBucket>(strand);
  storage_bucket->set_shared_bucket(get_shared_bucket(name));

  return storage_bucket;
}

void StrandStorageBucket::read(const string_& key, ReadCallback&& callback) {
  if (_owner_strand == nullptr) {
    LOG_ERROR("strand_storage", LOG << "strand is null");
    return;
  }
  auto read_callback =
      [ this, keepref = get_ref(), key, callback = std::move(callback) ](
          const string_& key, std::vector<uint8_t> data,
          BucketReadStatus read_status) mutable {
    _owner_strand->post([
      keepref = std::move(keepref),  // this capture is only to make sure it's
                                     // destroyed in strand
      callback = std::move(callback), key = std::move(key),
      data = std::move(data), read_status = std::move(read_status)
    ]() { callback(std::move(key), std::move(data), std::move(read_status)); });
  };
  _shared_bucket->read(key, read_callback);
}

void StrandStorageBucket::read_all(ReadCallback&& callback,
                                   std::function<void()>&& on_done) {
  if (_owner_strand == nullptr) {
    LOG_ERROR("strand_storage", LOG << "strand is null");
    return;
  }
  struct CallbackGroup {
    LinkStrandRef strand;
    ReadCallback callback;
    std::function<void()> on_done;
    CallbackGroup(const LinkStrandRef& strand, ReadCallback&& callback,
                  std::function<void()>&& on_done)
        : strand(strand),
          callback(std::move(callback)),
          on_done(std::move(on_done)) {}
  };
  auto callbackgroup = std::make_shared<CallbackGroup>(
      _owner_strand, std::move(callback), std::move(on_done));

  ReadCallback* unsafe_callback = &callbackgroup->callback;
  LinkStrand* unsafe_strand = callbackgroup->strand.get();

  // no need to keep_ref here, it's protected by on_done_callback
  auto read_callback = [this, unsafe_callback, unsafe_strand](
                           const string_& key, std::vector<uint8_t> data,
                           BucketReadStatus read_status) mutable {
    unsafe_strand->post([
      unsafe_callback, key = std::move(key), data = std::move(data),
      read_status = std::move(read_status)
    ]() {
      (*unsafe_callback)(std::move(key), std::move(data),
                         std::move(read_status));
    });

  };

  auto on_done_callback = [callbackgroup = std::move(callbackgroup)]() mutable {
    LinkStrand* p_strand = callbackgroup->strand.get();
    p_strand->post([callbackgroup = std::move(callbackgroup)]() {
      callbackgroup->on_done();
    });
  };

  _shared_bucket->read_all(std::move(read_callback),
                           std::move(on_done_callback));
}
}  // namespace dsa
