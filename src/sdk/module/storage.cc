#include "dsa_common.h"

#include "storage.h"

#include "module/default/simple_storage.h"
#include "util/string_encode.h"

namespace dsa {

StorageBucket& Storage::get_config_bucket() {
  static SimpleSafeStorageBucket config_bucket("config", nullptr, "");
  return config_bucket;
}

ref_<StrandStorageBucket> Storage::get_strand_bucket(
    const string_& name, const LinkStrandRef &strand) {
  ref_<StrandStorageBucket> storage_bucket =
      make_ref_<StrandStorageBucket>(strand);
  storage_bucket->set_shared_bucket(get_shared_bucket(name));

  return storage_bucket;
}

void StrandStorageBucket::read(const string_& key, ReadCallback&& callback) {
  auto read_callback =
      [ this, keepref = get_ref(), key, callback = std::move(callback) ](
          const string_& key, std::vector<uint8_t> data,
          BucketReadStatus read_status) mutable {
    if (_owner_strand != nullptr)
      _owner_strand->post([
        keepref = std::move(keepref),  // this capture is only to make sure it's
                                       // destroyed in strand
        callback = std::move(callback), key = std::move(key),
        data = std::move(data), read_status = std::move(read_status)
      ]() {
        callback(std::move(key), std::move(data), std::move(read_status));
      });
    else
      callback(std::move(key), std::move(data), std::move(read_status));
  };
  _shared_bucket->read(key, read_callback);
}

void StrandStorageBucket::read_all(ReadCallback&& callback,
                                   std::function<void()>&& on_done) {
  shared_ptr_<ReadCallback> callback_ptr =
      std::make_shared<ReadCallback>(std::move(callback));

  ReadCallback* unsafe_callback = callback_ptr.get();

  // no need to keep_ref here, it's protected by on_done_callback
  auto read_callback = [this, unsafe_callback](
                           const string_& key, std::vector<uint8_t> data,
                           BucketReadStatus read_status) mutable {
    if (_owner_strand != nullptr) {
      _owner_strand->post([
        unsafe_callback, key = std::move(key), data = std::move(data),
        read_status = std::move(read_status)
      ]() {
        (*unsafe_callback)(std::move(key), std::move(data),
                           std::move(read_status));
      });
    } else {
      (*unsafe_callback)(std::move(key), std::move(data),
                         std::move(read_status));
    }
  };

  auto on_done_callback = [
    this, keep_ref = get_ref(), callback_ptr = std::move(callback_ptr),
    on_done = std::move(on_done)
  ]() mutable {
    if (_owner_strand != nullptr) {
      _owner_strand->post([
        keep_ref = std::move(keep_ref), callback_ptr = std::move(callback_ptr),
        on_done = std::move(on_done)
      ]() { on_done(); });
    } else {
      on_done();
    }
  };

  _shared_bucket->read_all(std::move(read_callback),
                           std::move(on_done_callback));
}
}  // namespace dsa
