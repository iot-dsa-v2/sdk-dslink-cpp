#ifndef DSA_SDK_MODULE_SIMPLE_STORAGE_H
#define DSA_SDK_MODULE_SIMPLE_STORAGE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../storage.h"

#include <boost/asio/io_service.hpp>

#include <map>
#include <utility>
#include <list>

namespace dsa {

static constexpr char storage_default[] = "storage";
class SimpleQueueBucket : public QueueBucket {
 public:
  void push_back(const string_& key, BytesRef&& data) override;
  // when count = 0, remove all elements in the queue
  void remove_front(const string_& key, size_t count) override;

  void read_all(ReadCallback&& callback,
                std::function<void()>&& on_done) override;

  void remove_all() override;
};

class SimpleStorageBucket : public StorageBucket {
 private:
  typedef std::map<string_, boost::asio::io_service::strand*> StrandMap;

 protected:
  typedef std::pair<string_, boost::asio::io_service::strand*> StrandPair;
  StrandMap strand_map;
  boost::asio::io_service* _io_service;

  // cwd/storage_root/bucket_name/key
  string_ _cwd;
  string_ _storage_root;
  string_ _bucket_name;
  string_ _full_base_path;
#if defined(_WIN32) || defined(_WIN64)
  std::wstring get_storage_path(const string_& key = "");
#else
  string_ get_storage_path(const string_& key = "");
#endif

 public:
  SimpleStorageBucket(const string_& bucket_name,
                      boost::asio::io_service* io_service = nullptr,
                      const string_& storage_root = storage_default,
                      const string_& cwd = "");

  bool is_empty() override;
  bool exists(const string_& key) override;
  void write(const string_& key, BytesRef&& data,
             bool is_binary = false) override;
  void read(const string_& key, ReadCallback&& callback,
            bool is_binary = false) override;
  void remove(const string_& key) override;

  /// the callback might run asynchronously
  void read_all(ReadCallback&& callback,
                std::function<void()>&& on_done) override;

  void remove_all() override;
  void destroy() override;
};

class SimpleSafeStorageBucket : public SimpleStorageBucket {
 public:
  SimpleSafeStorageBucket(const string_& bucket_name,
                          boost::asio::io_service* io_service = nullptr,
                          const string_& storage_root = storage_default,
                          const string_& cwd = "")
      : SimpleStorageBucket(bucket_name, io_service, storage_root, cwd) {}

  void write(const string_& key, BytesRef&& data,
             bool is_binary = false) override;
  static SimpleSafeStorageBucket& get_config_bucket();
};

class SimpleStorage : public Storage {
 private:
  boost::asio::io_service* _io_service;
  std::list<shared_ptr_<StorageBucket>> _bucket_list;

 public:
  SimpleStorage(boost::asio::io_service* io_service = nullptr)
      : _io_service(io_service) {}

  shared_ptr_<StorageBucket> get_shared_bucket(const string_& name) override;

  /// create a bucket or find a existing bucket
  std::unique_ptr<QueueBucket> get_queue_bucket(const string_& name) override;

  void set_io_service(boost::asio::io_service* io_service) {
    _io_service = io_service;
  };

  void destroy_impl() final;
};

}  // namespace dsa

#endif  // DSA_SDK_MODULE_SIMPLE_STORAGE_H
