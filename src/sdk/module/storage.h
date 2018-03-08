#ifndef DSA_SDK_MODULE_STORAGE_H
#define DSA_SDK_MODULE_STORAGE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "dsa_common.h"
#include "core/link_strand.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/asio/strand.hpp>

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
  virtual void write(const string_& key, BytesRef&& data,
                     bool is_binary = false) = 0;
  virtual void read(const string_& key, ReadCallback&& callback,
                    bool is_binary = false) = 0;
  virtual void remove(const string_& key) = 0;

  /// the callback might run asynchronously
  virtual void read_all(ReadCallback&& callback,
                        std::function<void()>&& on_done) = 0;

  virtual void remove_all() = 0;
  virtual ~StorageBucket() = default;

  boost::asio::io_service::strand* get_owner_strand() { return owner_strand; };
  void set_owner_strand(boost::asio::io_service::strand* strand) { owner_strand = strand; };

 private:
  boost::asio::io_service::strand* owner_strand = nullptr;

};

class Storage : public DestroyableRef<Storage> {
 public:
  /// create a bucket or find a existing bucket
  virtual shared_ptr_<StorageBucket> get_shared_bucket(const string_& name) = 0;

  shared_ptr_<StorageBucket> get_strand_bucket(const string_& name, boost::asio::io_service::strand* strand);

  virtual bool queue_supported() { return false; }
  /// create a bucket or find a existing bucket
  virtual std::unique_ptr<QueueBucket> get_queue_bucket(
      const string_& name) = 0;

  virtual ~Storage() = default;
};

}  // namespace dsa

#endif  // DSA_SDK_MODULE_STORAGE_H
