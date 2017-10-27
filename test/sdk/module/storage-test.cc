#include "module/default/simple_storage.h"
#include "gtest/gtest.h"

#include <iostream>

using namespace dsa;

TEST(ModuleTest, StorageBucket) {

  SimpleStorage ss;

  std::string storage_key("config");

  StorageBucket& sb = ss.get_bucket(storage_key);

  auto read_callback = [&](std::string storage_key, std::vector<uint8_t> data) {
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

  sb.remove_all();
  

  // sb.remove(storage_key);
}
