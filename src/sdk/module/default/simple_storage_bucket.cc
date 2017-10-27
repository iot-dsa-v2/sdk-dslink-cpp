#include "simple_storage.h"

#include <fstream>
#include <iostream>

#include <boost/filesystem.hpp>

namespace dsa {

namespace fs = boost::filesystem;
using boost::filesystem::path;

static std::string storage_root = "/tmp/";

void SimpleStorageBucket::write(const std::string& key, BytesRef&& content) {
  path p(storage_root);
  p /= (key + ".data");

  std::ofstream ofs(p.string().c_str(), std::ios::out | std::ios::trunc);
  ofs.write(reinterpret_cast<const char*>(content->data()), content->size());

  return;
}

void SimpleStorageBucket::read(const std::string& key,
                               ReadCallback&& callback) {
  std::vector<uint8_t> vec{};

  path p(storage_root);
  p /= (key + ".data");

  size_t size = fs::file_size(p);

  std::ifstream ifs(p.string().c_str(), std::ios::in);

  if (size) {
    vec.resize(static_cast<size_t>(size));
    ifs.read(reinterpret_cast<char*>(&vec.front()), static_cast<size_t>(size));
  }

  callback(key, vec);

  return;
}

void SimpleStorageBucket::remove(const std::string& key) {
  path p(storage_root);
  p /= (key + ".data");

  if (exists(p) && is_regular_file(p)) {
    fs::remove(p);
  }

  return;
}

/// the callback might run asynchronously
void SimpleStorageBucket::read_all(ReadCallback&& callback,
                                   std::function<void()>&& on_done) {
  return;
}

void SimpleStorageBucket::remove_all() {
  path p(storage_root);

  try {
    std::vector<std::string> v;

    for (auto&& x : fs::directory_iterator(p)) {
      if (x.path().extension() == ".data") {
        v.push_back(x.path().filename().string());
      }
    }
  } catch (const fs::filesystem_error& ex) {
    // std::cout << ex.what() << '\n';
  }

  return;
}

}  // namespace dsa
