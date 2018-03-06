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
#if defined(_WIN32) || defined(_WIN64)
  static constexpr wchar_t storage_default[] = L"storage";
# else 
  static constexpr char	   storage_default[] = "storage";
# endif
class SimpleQueueBucket : public QueueBucket {

 public:
  void push_back(const wstring_& key, BytesRef&& data) override;
  // when count = 0, remove all elements in the queue
  void remove_front(const wstring_& key, size_t count) override;

  void read_all(ReadCallback&& callback,
                std::function<void()>&& on_done) override;

  void remove_all() override;
};

class SimpleStorageBucket : public StorageBucket {
 private:
  typedef std::map<wstring_, boost::asio::io_service::strand*> StrandMap;

 protected:
  typedef std::pair<wstring_, boost::asio::io_service::strand*> StrandPair;
  StrandMap strand_map;
  boost::asio::io_service* _io_service;

  // cwd/storage_root/bucket_name/key
  wstring_ _cwd;
  wstring_ _storage_root;
  wstring_ _bucket_name;
  wstring_ _full_base_path;
  wstring_ get_storage_path(const wstring_& key = empty_wstring);

 public:
  SimpleStorageBucket(const wstring_& bucket_name,
                      boost::asio::io_service* io_service = nullptr,
                      const wstring_& storage_root = storage_default,
                      const wstring_& cwd = empty_wstring);

  bool exists(const wstring_& key) override;
  void write(const wstring_& key, BytesRef&& data,
             bool is_binary = false) override;
  void read(const wstring_& key, ReadCallback&& callback,
            bool is_binary = false) override;
  void remove(const wstring_& key) override;

  /// the callback might run asynchronously
  void read_all(ReadCallback&& callback,
                std::function<void()>&& on_done) override;

  void remove_all() override;
};

class SimpleSafeStorageBucket : public SimpleStorageBucket {
 public:
  SimpleSafeStorageBucket(const wstring_& bucket_name,
                          boost::asio::io_service* io_service = nullptr,
                          const wstring_& storage_root = storage_default,
                          const wstring_& cwd = empty_wstring)
      : SimpleStorageBucket(bucket_name, io_service, storage_root, cwd) {}

  void write(const wstring_& key, BytesRef&& data,
             bool is_binary = false) override;
  static SimpleSafeStorageBucket& get_config_bucket();
};

class SimpleStorage : public Storage {
 private:
  boost::asio::io_service* _io_service;

 public:
  SimpleStorage(boost::asio::io_service* io_service = nullptr)
      : _io_service(io_service) {}

  std::unique_ptr<StorageBucket> get_bucket(const wstring_& name) override;

  /// create a bucket or find a existing bucket
  std::unique_ptr<QueueBucket> get_queue_bucket(
      const wstring_& name) override;

  void set_io_service(boost::asio::io_service* io_service) {
    _io_service = io_service;
  };
};

}  // namespace dsa

#endif  // DSA_SDK_MODULE_SIMPLE_STORAGE_H
