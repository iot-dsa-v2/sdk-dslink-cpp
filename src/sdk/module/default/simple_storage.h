#ifndef DSA_SDK_MODULE_SIMPLE_STORAGE_H
#define DSA_SDK_MODULE_SIMPLE_STORAGE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../storage.h"

#include <boost/asio/io_service.hpp>
#include <boost/filesystem/path.hpp>

#include <list>
#include <map>
#include <utility>

namespace fs = boost::filesystem;

namespace dsa {

// handle both utf8 file system and utf16 system
const string_ path_to_utf8_str(const boost::filesystem::path& path);
// handle both utf8 file system and utf16 system
fs::path utf8_str_to_path(const string_& str);

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
struct ReadCbTrack {
  int num_needed;  // how many is done now
  std::mutex read_mutex;
};

class SimpleStorageBucket : public SharedStorageBucket {
 private:
  typedef std::map<string_, boost::asio::io_service::strand*> StrandMap;
  std::mutex remove_mutex;

 protected:
  typedef std::pair<string_, boost::asio::io_service::strand*> StrandPair;
  StrandMap strand_map;
  boost::asio::io_service* _io_service;

  // cwd/storage_root/bucket_name/key
  string_ _cwd;
  string_ _storage_root;
  string_ _bucket_name;
  string_ _full_base_str;
  fs::path _full_base_path;

 public:
  SimpleStorageBucket(const string_& bucket_name,
                      boost::asio::io_service* io_service = nullptr,
                      const string_& storage_root = storage_default,
                      const string_& cwd = "");
  fs::path get_storage_path(const string_& key);
  bool is_empty() override;
  bool exists(const string_& key) override;
  void write(const string_& key, BytesRef&& data) override;
  void read(const string_& key, ReadCallback&& callback) override;
  void remove(const string_& key) override;

  /// the callback might run asynchronously
  void read_all(ReadCallback&& callback,
                std::function<void()>&& on_done) override;

  void remove_all() override;
  void destroy_bucket() override;
};

class SimpleSafeStorageBucket : public SimpleStorageBucket {
 public:
  SimpleSafeStorageBucket(const string_& bucket_name,
                          boost::asio::io_service* io_service = nullptr,
                          const string_& storage_root = storage_default,
                          const string_& cwd = "")
      : SimpleStorageBucket(bucket_name, io_service, storage_root, cwd) {}

  void write(const string_& key, BytesRef&& data) override;
  static SimpleSafeStorageBucket& get_config_bucket();
};

class SimpleStorage : public Storage {
 private:
  boost::asio::io_service* _io_service;
  std::list<shared_ptr_<SharedStorageBucket>> _bucket_list;

 public:
  SimpleStorage(boost::asio::io_service* io_service = nullptr)
      : _io_service(io_service) {}

  shared_ptr_<SharedStorageBucket> get_shared_bucket(
      const string_& name) override;

  /// create a bucket or find a existing bucket
  std::unique_ptr<QueueBucket> get_queue_bucket(const string_& name) override;

  void set_io_service(boost::asio::io_service* io_service) {
    _io_service = io_service;
  };

  void destroy_impl() final;
  void clear() final;
};

}  // namespace dsa

#endif  // DSA_SDK_MODULE_SIMPLE_STORAGE_H
