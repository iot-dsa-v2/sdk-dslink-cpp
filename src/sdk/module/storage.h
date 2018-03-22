#ifndef DSA_SDK_MODULE_STORAGE_H
#define DSA_SDK_MODULE_STORAGE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "core/link_strand.h"
#include "dsa_common.h"

#include <boost/asio.hpp>
#include <boost/asio/strand.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "util/buffer.h"
#include "util/enable_ref.h"

namespace dsa {
enum class BucketReadStatus : uint8_t {
  OK = 0x00,
  NOT_EXIST = 0x01,
  READ_FAILED = 0x02,
  FILE_OPEN_ERROR = 0x03
};
/// key->binary storage
class QueueBucket {
 public:
  typedef std::function<void(const string_& key, std::vector<uint8_t> data,
                             bool key_finished)>
      ReadCallback;

  virtual void push_back(const string_& key, BytesRef&& data) = 0;
  // when count = 0, remove all elements in the queue
  virtual void remove_front(const string_& key, size_t count) = 0;

  virtual void read_all(ReadCallback&& callback,
                        std::function<void()>&& on_done) = 0;

  virtual void remove_all() = 0;
};

/// key->binary storage
class StorageBucket {
 public:
  typedef std::function<void(const string_& key, std::vector<uint8_t> data,
                             BucketReadStatus read_status)>
      ReadCallback;
  virtual bool is_empty() = 0;
  virtual bool exists(const string_& key) = 0;
  virtual void write(const string_& key, BytesRef&& data) = 0;
  virtual void read(const string_& key, ReadCallback&& callback) = 0;
  virtual void remove(const string_& key) = 0;

  /// the callback might run asynchronously
  virtual void read_all(ReadCallback&& callback,
                        std::function<void()>&& on_done) = 0;

  virtual void remove_all() = 0;
  virtual ~StorageBucket() = default;
  virtual void destroy_bucket(){};
};

class StrandStorageBucket : public StorageBucket,
                            public DestroyableRef<StrandStorageBucket> {
 public:
  bool is_empty() final { return _shared_bucket->is_empty(); }
  bool exists(const string_& key) final { return _shared_bucket->exists(key); }
  void write(const string_& key, BytesRef&& data) final {
    _shared_bucket->write(key, std::move(data));
  }
  void read(const string_& key, ReadCallback&& callback) final;
  void remove(const string_& key) final { _shared_bucket->remove(key); }

  /// the callback might run asynchronously
  void read_all(ReadCallback&& callback, std::function<void()>&& on_done) final;

  void remove_all() final { _shared_bucket->remove_all(); }
  virtual ~StrandStorageBucket() = default;
  void destroy_impl() final {
    _owner_strand.reset();
    _shared_bucket->destroy_bucket();
  }

  StrandStorageBucket(const LinkStrandRef &strand) {
    set_owner_strand(strand);
  };
  LinkStrandRef get_owner_strand() { return _owner_strand; };
  void set_owner_strand(const LinkStrandRef &strand) {
    _owner_strand = strand;
  };
  void set_shared_bucket(shared_ptr_<StorageBucket>&& shared_bucket) {
    _shared_bucket = std::move(shared_bucket);
  }

 private:
  LinkStrandRef _owner_strand = nullptr;
  shared_ptr_<StorageBucket> _shared_bucket;
};

class Storage : public DestroyableRef<Storage> {
 public:
  // config bucket
  static StorageBucket& get_config_bucket();

  /// create a bucket or find a existing bucket
  virtual shared_ptr_<StorageBucket> get_shared_bucket(const string_& name) = 0;

  ref_<StrandStorageBucket> get_strand_bucket(const string_& name,
                                              const LinkStrandRef &strand);

  virtual bool queue_supported() { return false; }
  /// create a bucket or find a existing bucket
  virtual std::unique_ptr<QueueBucket> get_queue_bucket(
      const string_& name) = 0;

  virtual ~Storage() = default;
  void destroy_impl() override{};
  virtual void clear() = 0;
};

}  // namespace dsa

#endif  // DSA_SDK_MODULE_STORAGE_H
