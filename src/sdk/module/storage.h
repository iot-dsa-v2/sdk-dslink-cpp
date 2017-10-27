#ifndef DSA_SDK_MODULE_STORAGE_H_
#define DSA_SDK_MODULE_STORAGE_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <functional>
#include <string>
#include <vector>

#include "dsa_common.h"
#include "util/buffer.h"

namespace dsa {

/// key->binary storage
class QueueBucket {
 public:
  typedef std::function<void(const std::string& key, std::vector<uint8_t> data,
                             bool key_finished)>
      ReadCallback;

  virtual void push_back(const std::string& key, BytesRef&& data) = 0;
  // when count = 0, remove all elements in the queue
  virtual void remove_front(const std::string& key, size_t count) = 0;

  virtual void read_all(ReadCallback&& callback,
                        std::function<void()>&& on_done) = 0;

  virtual void remove_all() = 0;
};

/// key->binary storage
class StorageBucket {
 public:
  typedef std::function<void(const std::string& key, std::vector<uint8_t> data)>
      ReadCallback;

  virtual void write(const std::string& key, BytesRef&& data) = 0;
  virtual void read(const std::string& key, ReadCallback&& callback) = 0;
  virtual void remove(const std::string& key) = 0;

  /// the callback might run asynchronously
  virtual void read_all(ReadCallback&& callback,
                        std::function<void()>&& on_done) = 0;

  virtual void remove_all() = 0;
};

class Storage {
 public:
  /// create a bucket or find a existing bucket
  virtual StorageBucket& get_bucket(const std::string& name) = 0;

  virtual bool queue_supported() { return false; }
  /// create a bucket or find a existing bucket
  virtual QueueBucket& get_queue_bucket(const std::string& name) = 0;
};

}  // namespace dsa

#endif  // DSA_SDK_MODULE_STORAGE_H_
