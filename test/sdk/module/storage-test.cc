﻿#include "dsa_common.h"

#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <codecvt>
#include <cstring>
#include <functional>
#include <iomanip>
#include <locale>
#include <map>
#include <string>
#include <vector>
#include "../async_test.h"
#include "../test_config.h"
#include "module/default/simple_storage.h"
#include "util/app.h"

using namespace dsa;
namespace fs = boost::filesystem;

TEST(ModuleTest, StorageBucket) {
  App app;

  SimpleStorage simple_storage(&app.io_service());

  string_ storage_key(u8"parent/sample_key周");
  string_ bucket_name(u8"bucket周");

  shared_ptr_<StorageBucket> storage_bucket =
      simple_storage.get_shared_bucket(bucket_name);

  auto on_done = []() {
    std::cout << "on_done" << std::endl;
    return;
  };

  {
    const char* content = {"first_item"};
    auto data = new RefCountBytes(&content[0], &content[strlen(content)]);
    storage_bucket->write(storage_key, std::forward<RefCountBytes*>(data));

    auto read_callback = [=](string_ storage_key, std::vector<uint8_t> vec,
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

    auto read_callback = [=](string_ storage_key, std::vector<uint8_t> vec,
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
  string_ storage_key(u8"sample_config周");
  string_ bucket_name(u8"config");

  shared_ptr_<StorageBucket> storage_bucket =
      simple_storage.get_shared_bucket(bucket_name);

  auto on_done = []() {
    std::cout << "on_done" << std::endl;
    return;
  };

  {
    const char* content = {"first_item"};
    auto data = new RefCountBytes(&content[0], &content[strlen(content)]);
    storage_bucket->write(storage_key, std::forward<RefCountBytes*>(data));

    auto read_callback = [=](string_ storage_key, std::vector<uint8_t> vec,
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

    auto read_callback = [=](string_ storage_key, std::vector<uint8_t> vec,
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
  const string_ storage_key1(u8"parent周");
  const string_ storage_key2(u8"parent周/sample_key1周");
  const string_ bucket_name(u8"bucket周");

  const char* content1 = {"first_item"};
  const char* content2 = {"second_item"};

  shared_ptr_<StorageBucket> storage_bucket =
      simple_storage.get_shared_bucket(bucket_name);

  auto on_done = []() {
    std::cout << "on_done" << std::endl;
    return;
  };

  {
    auto data = new RefCountBytes(&content1[0], &content1[strlen(content1)]);
    storage_bucket->write(storage_key1, std::forward<RefCountBytes*>(data));

    auto read_callback = [=](string_ storage_key, std::vector<uint8_t> vec,
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

    auto read_callback = [=](string_ storage_key, std::vector<uint8_t> vec,
                             BucketReadStatus read_status) {
      EXPECT_EQ(read_status, BucketReadStatus::OK);
      EXPECT_EQ(0, strncmp(content2, reinterpret_cast<const char*>(vec.data()),
                           vec.size()));

      return;
    };
    storage_bucket->read(storage_key2, read_callback);
  }
  int read_order = 0;
  auto read_all_callback = [&](string_ storage_key, std::vector<uint8_t> vec,
                               BucketReadStatus read_status) {
    EXPECT_EQ(read_status, BucketReadStatus::OK);
    if (read_order == 0) {
      EXPECT_EQ(0, strncmp(content1, reinterpret_cast<const char*>(vec.data()),
                           vec.size()));
      EXPECT_EQ(storage_key, storage_key1);
    } else if (read_order == 1) {
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
  EXPECT_TRUE(storage_bucket->is_empty());
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

  string_ storage_key(u8"parent/sample_key呀");
  string_ bucket_name(u8"bucket功");

  shared_ptr_<StorageBucket> storage_bucket =
      simple_storage.get_shared_bucket(bucket_name);

  auto on_done = []() {
    std::cout << "on_done" << std::endl;
    return;
  };

  {
    const char* content = {"first_item"};
    auto data = new RefCountBytes(&content[0], &content[strlen(content)]);
    storage_bucket->write(storage_key, std::forward<RefCountBytes*>(data));

    auto read_callback = [=](string_ storage_key, std::vector<uint8_t> vec,
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

TEST(ModuleTest, StrandBucket) {
  auto app = std::make_shared<App>();

  TestConfig server_strand(app, false);

  SimpleStorage simple_storage(&app->io_service());

  std::string storage_key("parent/sample_key");

  auto storage_bucket =
      simple_storage.get_strand_bucket("bucket", server_strand.strand);

  int cnt = 0;
  server_strand.strand->post([&]() {
    for (cnt = 0; cnt < 2; cnt++)
      boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
  });

  EXPECT_TRUE(cnt < 2);

  bool read_cb_called = false;
  {
    const char* content = {"first_item"};
    auto data = new RefCountBytes(&content[0], &content[strlen(content)]);
    storage_bucket->write(storage_key, std::forward<RefCountBytes*>(data));

    auto read_callback = [&](std::string storage_key, std::vector<uint8_t> vec,
                             BucketReadStatus read_status) {
      EXPECT_EQ(cnt, 2);
      EXPECT_EQ(read_status, BucketReadStatus::OK);
      EXPECT_EQ(0, strncmp(content, reinterpret_cast<const char*>(vec.data()),
                           vec.size()));
      read_cb_called = true;
      return;
    };
    storage_bucket->read(storage_key, read_callback);
  }
  ASYNC_EXPECT_TRUE(3000, *server_strand.strand,
                    [&]() -> bool { return read_cb_called; });
  server_strand.strand->post([&]() { storage_bucket->remove_all(); });
  ASYNC_EXPECT_TRUE(3000, *server_strand.strand,
                    [&]() -> bool { return storage_bucket->is_empty(); });

  server_strand.strand->post([&]() { storage_bucket->destroy(); });

  server_strand.destroy();
  app->close();

  wait_for_bool(500, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  app->wait();
}

TEST(ModuleTest, StrandBucketReadAll) {
  auto app = std::make_shared<App>();

  TestConfig server_strand(app, false);

  SimpleStorage simple_storage(nullptr);
  // on_done is not guaranteed when simple storage runs async, since all keys
  // have their own strand and read callbacks are posted on them
  // SimpleStorage simple_storage(&app->io_service());

  const string_ storage_key1(u8"parent周");
  const string_ storage_key2(u8"parent周/sample_key1周");
  const string_ bucket_name(u8"bucketA");

  const char* content1 = {"first_item"};
  const char* content2 = {"second_item"};

  auto storage_bucket =
      simple_storage.get_strand_bucket("bucketA", server_strand.strand);

  bool read_done = false;

  {
    auto data = new RefCountBytes(&content1[0], &content1[strlen(content1)]);
    storage_bucket->write(storage_key1, std::forward<RefCountBytes*>(data));

    auto read_callback = [&](string_ storage_key, std::vector<uint8_t> vec,
                             BucketReadStatus read_status) {
      EXPECT_EQ(read_status, BucketReadStatus::OK);
      EXPECT_EQ(0, strncmp(content1, reinterpret_cast<const char*>(vec.data()),
                           vec.size()));
      read_done = true;
      return;
    };
    storage_bucket->read(storage_key1, read_callback);
  }
  ASYNC_EXPECT_TRUE(1000, *server_strand.strand,
                    [&]() -> bool { return read_done; });
  read_done = false;
  {
    auto data = new RefCountBytes(&content2[0], &content2[strlen(content2)]);
    storage_bucket->write(storage_key2, std::forward<RefCountBytes*>(data));

    auto read_callback = [&](string_ storage_key, std::vector<uint8_t> vec,
                             BucketReadStatus read_status) {
      EXPECT_EQ(read_status, BucketReadStatus::OK);
      EXPECT_EQ(0, strncmp(content2, reinterpret_cast<const char*>(vec.data()),
                           vec.size()));
      read_done = true;
      return;
    };
    storage_bucket->read(storage_key2, read_callback);
  }
  ASYNC_EXPECT_TRUE(1000, *server_strand.strand,
                    [&]() -> bool { return read_done; });
  read_done = false;
  int read_order = 0;
  auto read_all_callback = [&](string_ storage_key, std::vector<uint8_t> vec,
                               BucketReadStatus read_status) {
    EXPECT_EQ(read_status, BucketReadStatus::OK);
    if (read_order == 0) {
      EXPECT_EQ(0, strncmp(content1, reinterpret_cast<const char*>(vec.data()),
                           vec.size()));
      EXPECT_EQ(storage_key, storage_key1);
    } else if (read_order == 1) {
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
    read_done = true;
    return;
  });

  ASYNC_EXPECT_TRUE(3000, *server_strand.strand,
                    [&]() -> bool { return read_done; });
  server_strand.strand->post([&]() { storage_bucket->remove_all(); });
  ASYNC_EXPECT_TRUE(3000, *server_strand.strand,
                    [&]() -> bool { return storage_bucket->is_empty(); });

  server_strand.strand->post([&]() { storage_bucket->destroy(); });

  server_strand.destroy();
  app->close();

  wait_for_bool(500, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  app->wait();
}
