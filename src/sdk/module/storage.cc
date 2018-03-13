#include "dsa_common.h"

#include "storage.h"
#include "util/string_encode.h"

namespace dsa {

ref_<StrandStorageBucket> Storage::get_strand_bucket(
    const string_& name, const LinkStrandRef& strand) {
  ref_<StrandStorageBucket> storage_bucket =
      make_ref_<StrandStorageBucket>(strand);
  storage_bucket->set_shared_bucket(get_shared_bucket(name));

  return storage_bucket;
}

void StrandStorageBucket::read(const string_& key, ReadCallback&& callback) {
  auto read_callback = [ =, callback = std::move(callback) ](
      const string_& key, std::vector<uint8_t> data,
      BucketReadStatus read_status) {
    if (_owner_strand != nullptr)
      _owner_strand->post([
        callback = std::move(callback), decode_key = std::move(url_decode(key)),
        data = std::move(data), read_status = std::move(read_status)
      ]() {
        callback(std::move(decode_key), std::move(data),
                 std::move(read_status));
      });
    else
      callback(std::move(url_decode(key)), std::move(data),
               std::move(read_status));
  };
  _shared_bucket->read(key, read_callback);
}

void StrandStorageBucket::read_all(ReadCallback&& callback,
                                   std::function<void()>&& on_done) {
  auto read_callback = [ =, callback = std::move(callback) ](
      const string_& key, std::vector<uint8_t> data,
      BucketReadStatus read_status) {
    if (_owner_strand != nullptr)
      _owner_strand->post([
        callback = std::move(callback), decode_key = std::move(url_decode(key)),
        data = std::move(data), read_status = std::move(read_status)
      ]() {
        callback(std::move(decode_key), std::move(data),
                 std::move(read_status));
      });
    else
      callback(std::move(url_decode(key)), std::move(data),
               std::move(read_status));
  };
  _shared_bucket->read_all(
      read_callback, [ this, keepref = get_ref(), on_done = std::move(on_done) ]() {
        if (_owner_strand != nullptr)
          _owner_strand->post([on_done = std::move(on_done)]() { on_done(); });
        else
          on_done();
      });
}
}