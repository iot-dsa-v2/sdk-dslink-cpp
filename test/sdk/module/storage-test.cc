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

  //TODO: make easier way
#if defined(_WIN32) || defined(_WIN64)
  wstring_ storage_key(L"parent/sample_key");
  wstring_ bucket_name(L"bucket");
#else
  wstring_ storage_key("parent/sample_key");
  wstring_ bucket_name("bucket");
#endif

  std::unique_ptr<StorageBucket> storage_bucket =
      simple_storage.get_bucket(bucket_name);

  auto on_done = []() {
    std::cout << "on_done" << std::endl;
    return;
  };

  {
    const char* content = {"first_item"};
    auto data = new RefCountBytes(&content[0], &content[strlen(content)]);
    storage_bucket->write(storage_key, std::forward<RefCountBytes*>(data));

    auto read_callback = [=](wstring_ storage_key, std::vector<uint8_t> vec,
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

	auto read_callback = [=](wstring_ storage_key, std::vector<uint8_t> vec,
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
#if defined(_WIN32) || defined(_WIN64)
  wstring_ storage_key(L"sample_config");
  wstring_ bucket_name(L"config");
#else
  wstring_ storage_key("sample_config");
  wstring_ bucket_name("config");
#endif

  std::unique_ptr<StorageBucket> storage_bucket =
      simple_storage.get_bucket(bucket_name);

  auto on_done = []() {
    std::cout << "on_done" << std::endl;
    return;
  };

  {
    const char* content = {"first_item"};
    auto data = new RefCountBytes(&content[0], &content[strlen(content)]);
    storage_bucket->write(storage_key, std::forward<RefCountBytes*>(data));

    auto read_callback = [=](wstring_ storage_key, std::vector<uint8_t> vec,
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

    auto read_callback = [=](wstring_ storage_key, std::vector<uint8_t> vec,
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

TEST(ModuleTest, StorageBucketReadAll) {
  App app;

  SimpleStorage simple_storage(nullptr);
#if defined(_WIN32) || defined(_WIN64)
  wstring_ storage_key1(L"parent");
  wstring_ storage_key2(L"parent/sample_key1");
  wstring_ bucket_name(L"bucket");
#else
  wstring_ storage_key1("parent");
  wstring_ storage_key2("parent/sample_key1");
  wstring_ bucket_name("bucket");
#endif

  const char* content1 = {"first_item"};
  const char* content2 = {"second_item"};

  std::unique_ptr<StorageBucket> storage_bucket =
      simple_storage.get_bucket(bucket_name);

  auto on_done = []() {
    std::cout << "on_done" << std::endl;
    return;
  };

  {
    auto data = new RefCountBytes(&content1[0], &content1[strlen(content1)]);
    storage_bucket->write(storage_key1, std::forward<RefCountBytes*>(data));

    auto read_callback = [=](wstring_ storage_key, std::vector<uint8_t> vec,
                             BucketReadStatus read_status) {

      EXPECT_EQ(read_status, BucketReadStatus::OK);
      EXPECT_EQ(0, strncmp(content1, reinterpret_cast<const char*>(vec.data()),
                           vec.size()));

      return;
    };
    storage_bucket->read(storage_key1, read_callback);
  }

  {
    auto data = new RefCountBytes(&content2[0], &content2[strlen(content2)]);
    storage_bucket->write(storage_key2, std::forward<RefCountBytes*>(data));

    auto read_callback = [=](wstring_ storage_key, std::vector<uint8_t> vec,
                             BucketReadStatus read_status) {
      EXPECT_EQ(read_status, BucketReadStatus::OK);
      EXPECT_EQ(0, strncmp(content2, reinterpret_cast<const char*>(vec.data()),
                           vec.size()));

      return;
    };
    storage_bucket->read(storage_key2, read_callback);
  }
  int read_order = 0;
  auto read_all_callback = [&](wstring_ storage_key, std::vector<uint8_t> vec,
                           BucketReadStatus read_status) {
    EXPECT_EQ(read_status, BucketReadStatus::OK);
    if(read_order == 0) {
      EXPECT_EQ(0, strncmp(content1, reinterpret_cast<const char*>(vec.data()),
                           vec.size()));
      EXPECT_EQ(storage_key, storage_key1);
    } else if(read_order == 1) {
      EXPECT_EQ(0, strncmp(content2, reinterpret_cast<const char*>(vec.data()),
                           vec.size()));
      EXPECT_EQ(storage_key, storage_key2);
    } else {
      EXPECT_TRUE(0);
    }
    read_order++;
    return;
  };
  storage_bucket->read_all(read_all_callback, [&]() {
    EXPECT_EQ(read_order, 2);
    return;
  });

  storage_bucket->remove_all();
  app.close();

  wait_for_bool(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}
TEST(ModuleTest, StorageBucketName) {
  App app;

  SimpleStorage simple_storage(&app.io_service());

  //TODO: make easier way
#if defined(_WIN32) || defined(_WIN64)
  wstring_ storage_key(L"parent/sample_key呀");
  wstring_ bucket_name(L"bucket功");
#else
  wstring_ storage_key("parent/sample_key呀");
  wstring_ bucket_name("bucket功");
#endif

  std::unique_ptr<StorageBucket> storage_bucket =
	simple_storage.get_bucket(bucket_name);

  auto on_done = []() {
	std::cout << "on_done" << std::endl;
	return;
  };

  {
	const char* content = { "first_item" };
	auto data = new RefCountBytes(&content[0], &content[strlen(content)]);
	storage_bucket->write(storage_key, std::forward<RefCountBytes*>(data));

	auto read_callback = [=](wstring_ storage_key, std::vector<uint8_t> vec,
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