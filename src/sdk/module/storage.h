#ifndef DSA_SDK_MODULE_STORAGE_H_
#define DSA_SDK_MODULE_STORAGE_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <functional>
#include <string>
#include <vector>

namespace dsa {

/// key->binary storage
class QueueBucket {
 public:
  typedef std::function<void(const std::string& key, std::vector<uint8_t> data,
                             bool key_finished)>
      ReadCallback;

  virtual void array_push_back(const std::string& key, const uint8_t* data,
                               size_t size) = 0;
  virtual void array_remove_front(const std::string& key, size_t count) = 0;

  virtual void read_all(ReadCallback&& callback,
                        std::function<void()>&& on_done) = 0;
};

/// key->binary storage
class StorageBucket {
 public:
  typedef std::function<void(const std::string& key, std::vector<uint8_t> data)>
      ReadCallback;

  virtual void write(const std::string& key, const uint8_t* data,
                     size_t size) = 0;
  virtual std::vector<uint8_t> read(const std::string& name) = 0;
  virtual void remove(const std::string& key) = 0;

  /// the callback might run asynchronously
  virtual void read_all(ReadCallback&& callback,
                        std::function<void()>&& on_done) = 0;
};

class Storage {
 public:
  /// create a bucket or find a existing bucket
  virtual StorageBucket& get_bucket(const std::string& name) = 0;
  /// delete everything in the bucket
  virtual void remove_bucket(const std::string& name) = 0;

  virtual bool queue_supported() { return false; }
  /// create a bucket or find a existing bucket
  virtual QueueBucket& get_queue_bucket(const std::string& name) = 0;
  /// delete everything in the bucket
  virtual void remove_queue_bucket(const std::string& name) = 0;
};

}  // namespace dsa

#endif  // DSA_SDK_MODULE_STORAGE_H_
