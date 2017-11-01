#include "module/default/simple_storage.h"
#include "gtest/gtest.h"

#include "core/app.h"

#include <iostream>

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>

#include <functional>
#include <map>
#include <string>

#include "../test/sdk/async_test.h"

using namespace dsa;

namespace ba = boost::asio;

TEST(ModuleTest, StorageBucket) {

  App app;

  SimpleStorage ss(&app.io_service());

  std::string storage_key("config");

  StorageBucket& sb = ss.get_bucket(storage_key);

  auto read_callback = [&](std::string storage_key, std::vector<uint8_t> data) {
    std::cout << "read_callback..." << std::endl;
    for (auto elem : data) {
      std::cout << elem;
    }
    std::cout << std::endl;

    return;
  };

  auto on_done = []() {
    std::cout << "on_done" << std::endl;
    return;
  };

  {
    const char* content = {"first_item"};
    auto data = new RefCountBytes(&content[0], &content[strlen(content)]);
    sb.write(storage_key, data);
  }

  sb.read(storage_key, read_callback);

  {
    const char* content = {"second_item"};
    auto data = new RefCountBytes(&content[0], &content[strlen(content)]);
    sb.write(storage_key, data);
  }

  sb.read_all(read_callback, on_done);

  storage_key.assign("config1");

  {
    const char* content = {"first_item"};
    auto data = new RefCountBytes(&content[0], &content[strlen(content)]);
    sb.write(storage_key, data);
  }

  sb.read_all(read_callback, on_done);
  sb.remove_all();

  wait_for_bool(500, [&]() { return false; });

  app.close();

  wait_for_bool(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}
