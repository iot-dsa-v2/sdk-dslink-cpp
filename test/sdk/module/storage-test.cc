#include "dsa_common.h"

#include <gtest/gtest.h>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <cstring>
#include <functional>
#include <map>
#include <string>
#include "../test/sdk/async_test.h"
#include "module/default/simple_storage.h"
#include "util/app.h"

using namespace dsa;

TEST(ModuleTest, StorageBucket) {
  App app;

  SimpleStorage simple_storage(&app.io_service());

  std::string storage_key("sample_key");

  std::unique_ptr<StorageBucket> storage_bucket =
      simple_storage.get_bucket("bucket");

  auto on_done = []() {
    std::cout << "on_done" << std::endl;
    return;
  };

  {
    const char* content = {"first_item"};
    auto data = new RefCountBytes(&content[0], &content[strlen(content)]);
    storage_bucket->write(storage_key, std::forward<RefCountBytes*>(data));

    auto read_callback = [=](std::string storage_key, std::vector<uint8_t> vec,
                             BucketReadStatus read_status) {

      EXPECT_EQ(read_status, BucketReadStatus::OK);
      EXPECT_EQ(0, strncmp(content, reinterpret_cast<const char*>(vec.data()),
                           vec.size()));

      return;
    };
    storage_bucket->read(storage_key, read_callback);
  }

  {
    const char* content = {"second_item"};
    auto data = new RefCountBytes(&content[0], &content[strlen(content)]);
    storage_bucket->write(storage_key, std::forward<RefCountBytes*>(data));

    auto read_callback = [=](std::string storage_key, std::vector<uint8_t> vec,
                             BucketReadStatus read_status) {
      EXPECT_EQ(read_status, BucketReadStatus::OK);
      EXPECT_EQ(0, strncmp(content, reinterpret_cast<const char*>(vec.data()),
                           vec.size()));

      return;
    };
    storage_bucket->read(storage_key, read_callback);
  }

  storage_bucket->remove(storage_key);

  app.close();

  wait_for_bool(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}

TEST(ModuleTest, SafeStorageBucket) {
  App app;

  SimpleStorage simple_storage(&app.io_service());

  std::string storage_key("sample_config");

  std::unique_ptr<StorageBucket> storage_bucket =
      simple_storage.get_bucket("config");

  auto on_done = []() {
    std::cout << "on_done" << std::endl;
    return;
  };

  {
    const char* content = {"first_item"};
    auto data = new RefCountBytes(&content[0], &content[strlen(content)]);
    storage_bucket->write(storage_key, std::forward<RefCountBytes*>(data));

    auto read_callback = [=](std::string storage_key, std::vector<uint8_t> vec,
                             BucketReadStatus read_status) {
      EXPECT_EQ(read_status, BucketReadStatus::OK);
      EXPECT_EQ(0, strncmp(content, reinterpret_cast<const char*>(vec.data()),
                           vec.size()));

      return;
    };
    storage_bucket->read(storage_key, read_callback);
  }

  {
    const char* content = {"second_item"};
    auto data = new RefCountBytes(&content[0], &content[strlen(content)]);
    storage_bucket->write(storage_key, std::forward<RefCountBytes*>(data));

    auto read_callback = [=](std::string storage_key, std::vector<uint8_t> vec,
                             BucketReadStatus read_status) {
      EXPECT_EQ(read_status, BucketReadStatus::OK);
      EXPECT_EQ(0, strncmp(content, reinterpret_cast<const char*>(vec.data()),
                           vec.size()));

      return;
    };
    storage_bucket->read(storage_key, read_callback);
  }

  storage_bucket->remove(storage_key);

  app.close();

  wait_for_bool(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}
