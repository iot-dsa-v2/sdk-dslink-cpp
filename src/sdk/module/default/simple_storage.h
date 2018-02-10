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
  bool _is_binary;

 public:
  SimpleStorageBucket() {}

  SimpleStorageBucket(boost::asio::io_service* io_service, bool is_binary = false)
      : _io_service(io_service), _is_binary(is_binary){}

  void write(const std::string& key, BytesRef&& data) override;
  void read(const std::string& key, ReadCallback&& callback) override;
  void remove(const std::string& key) override;

  /// the callback might run asynchronously
  void read_all(ReadCallback&& callback,
                std::function<void()>&& on_done) override;

  void remove_all() override;
};

class SimpleSafeStorageBucket : public SimpleStorageBucket {
 public:
  SimpleSafeStorageBucket() {}

  SimpleSafeStorageBucket(boost::asio::io_service* io_service, bool is_binary = false)
  : SimpleStorageBucket(io_service, is_binary) {}

  void write(const std::string& key, BytesRef&& data) override;
};

class SimpleStorage : public Storage {
 private:
  boost::asio::io_service* _io_service;

 public:
  SimpleStorage(boost::asio::io_service* io_service = nullptr)
      : _io_service(io_service) {}

  std::unique_ptr<StorageBucket> get_bucket(const std::string& name) override;

  /// create a bucket or find a existing bucket
  std::unique_ptr<QueueBucket> get_queue_bucket(const std::string& name) override;

  void set_io_service(boost::asio::io_service* io_service){
    _io_service = io_service;
  };
};

}  // namespace dsa

#endif  // DSA_SDK_MODULE_SIMPLE_STORAGE_H
