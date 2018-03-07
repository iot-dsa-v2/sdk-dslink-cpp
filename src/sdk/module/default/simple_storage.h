#ifndef DSA_SDK_MODULE_SIMPLE_STORAGE_H
#define DSA_SDK_MODULE_SIMPLE_STORAGE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../storage.h"

#include <boost/asio/io_service.hpp>

#include <map>
#include <utility>

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
 private:
  typedef std::map<std::string, boost::asio::io_service::strand*> StrandMap;

 protected:
  typedef std::pair<std::string, boost::asio::io_service::strand*> StrandPair;
  StrandMap strand_map;
  boost::asio::io_service* _io_service;

  // cwd/storage_root/bucket_name/key
  string_ _cwd;
  string_ _storage_root;
  string_ _bucket_name;
  string_ _full_base_path;
  string_ get_storage_path(const string_& key = "");

 public:
  SimpleStorageBucket(const string_& bucket_name,
                      boost::asio::io_service* io_service = nullptr,
                      const string_& storage_root = "storage",
                      const string_& cwd = "" );
  bool exists (const string_ &key) override;
  void write(const std::string& key, BytesRef&& data,
             bool is_binary = false) override;
  void read(const std::string& key, ReadCallback&& callback,
            bool is_binary = false) override;
  void remove(const std::string& key) override;

  /// the callback might run asynchronously
  void read_all(ReadCallback&& callback,
                std::function<void()>&& on_done) override;

  void remove_all() override;
};

class SimpleSafeStorageBucket : public SimpleStorageBucket {
 public:
  SimpleSafeStorageBucket(const string_& bucket_name,
                          boost::asio::io_service* io_service = nullptr,
                          const string_& storage_root = "storage",
                          const string_& cwd = "" )
      : SimpleStorageBucket(bucket_name, io_service, storage_root, cwd) {}

  void write(const std::string& key, BytesRef&& data,
             bool is_binary = false) override;
  static SimpleSafeStorageBucket& get_config_bucket();
};

class SimpleStorage : public Storage {
 private:
  boost::asio::io_service* _io_service;

 public:
  SimpleStorage(boost::asio::io_service* io_service = nullptr)
      : _io_service(io_service) {}

  std::unique_ptr<StorageBucket> get_bucket(const std::string& name) override;

  /// create a bucket or find a existing bucket
  std::unique_ptr<QueueBucket> get_queue_bucket(
      const std::string& name) override;

  void set_io_service(boost::asio::io_service* io_service) {
    _io_service = io_service;
  };
};

}  // namespace dsa

#endif  // DSA_SDK_MODULE_SIMPLE_STORAGE_H
