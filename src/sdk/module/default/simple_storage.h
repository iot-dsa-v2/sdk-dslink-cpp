#ifndef DSA_SDK_MODULE_SIMPLE_STORAGE_H_
#define DSA_SDK_MODULE_SIMPLE_STORAGE_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include "../storage.h"

namespace dsa {

class SimpleQueueBucket : public QueueBucket {
public:
  void push_back(const std::string& key, BytesRef&& data) override;
  // when count = 0, remove all elements in the queue
  void remove_front(const std::string& key, size_t count) override;

  void read_all(ReadCallback&& callback,
                        std::function<void()>&& on_done) override;

  void remove_all() override;
};

class SimpleStorageBucket : public StorageBucket {
public:
  void write(const std::string& key, BytesRef&& data) override;
  void read(const std::string& key, ReadCallback&& callback) override;
  void remove(const std::string& key) override;

  /// the callback might run asynchronously
  void read_all(ReadCallback&& callback,
                        std::function<void()>&& on_done) override;

  void remove_all() override;
};

class SimpleStorage : public Storage {
public:
  StorageBucket& get_bucket(const std::string& name) override;

  /// create a bucket or find a existing bucket
  QueueBucket& get_queue_bucket(const std::string& name) override;
};

}  // namespace dsa

#endif  // DSA_SDK_MODULE_SIMPLE_STORAGE_H_
