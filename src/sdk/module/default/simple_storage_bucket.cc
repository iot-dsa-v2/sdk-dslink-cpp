#include "dsa_common.h"

#include "simple_storage.h"

#include <boost/asio/strand.hpp>
#include <boost/filesystem.hpp>
#include <fstream>

namespace dsa {

namespace fs = boost::filesystem;
using boost::filesystem::path;

#if (defined(_WIN32) || defined(_WIN64))
static std::string storage_root = "C:\\temp\\";
#else
//static std::string storage_root = "/tmp/";
static std::string storage_root = "";
#endif

void SimpleStorageBucket::write(const std::string& key, BytesRef&& content) {

  auto write_file = [=] () {
    path p(storage_root);
    p /= (key);

    try {
      std::ofstream ofs(p.string().c_str(), std::ios::out | std::ios::trunc);
      if (ofs) {
        ofs.write(reinterpret_cast<const char*>(content->data()),
                  content->size());
      } else {
        // TODO - error handling
      }
    } catch (const fs::filesystem_error& ex) {
      // TODO - error handling
    }};

  if (_io_service != nullptr) {
    if (!strand_map.count(key)) {
      boost::asio::io_service::strand* strand =
          new boost::asio::io_service::strand(*_io_service);
      strand_map.insert(StrandPair(key, strand));
    }

    strand_map.at(key)->post([=]() {
      write_file();
      return;
    });
    return;
  } else {
    write_file();
  }
}

void SimpleStorageBucket::read(const std::string& key,
                               ReadCallback&& callback) {
  auto read_file = [=] () {
    std::vector<uint8_t> vec{};

    path p(storage_root);
    p /= (key);

    try {
      if (fs::exists(p) && is_regular_file(p)) {
        size_t size = fs::file_size(p);

        if (size) {
          std::ifstream ifs(p.string().c_str(), std::ios::in);
          if (ifs) {
            vec.resize(static_cast<size_t>(size));
            ifs.read(reinterpret_cast<char*>(&vec.front()),
                     static_cast<size_t>(size));
          } else {
            // TODO - error handling
          }
        }
      } else {
        // TODO - error handling
      }
    } catch (const fs::filesystem_error& ex) {
      // TODO - error handling
    }

    callback(key, vec);
  };

  if (_io_service != nullptr) {
    if (!strand_map.count(key)) {
      boost::asio::io_service::strand* strand =
          new boost::asio::io_service::strand(*_io_service);
      strand_map.insert(StrandPair(key, strand));
    }

    strand_map.at(key)->post([=]() {
      read_file();
      return;
    });
    return;
  } else {
    read_file();
  }
}

void SimpleStorageBucket::remove(const std::string& key) {
  if (_io_service != nullptr) {
    if (!strand_map.count(key)) {
      boost::asio::io_service::strand* strand =
          new boost::asio::io_service::strand(*_io_service);
      strand_map.insert(StrandPair(key, strand));
    }

    strand_map.at(key)->post([=]() {
      path p(storage_root);
      p /= (key);

      try {
        if (exists(p) && is_regular_file(p)) {
          fs::remove(p);
          delete strand_map.at(key);
          strand_map.erase(key);  // erase the element from the map as well, if
                                  // it is removed successfully
        }
      } catch (const fs::filesystem_error& ex) {
        ;
      }

      return;
    });

    return;
  } else {
    path p(storage_root);
    p /= (key);

    try {
      if (exists(p) && is_regular_file(p)) {
        fs::remove(p);
      }
    } catch (const fs::filesystem_error& ex) {
      ;
    }
  }
}

/// the callback might run asynchronously
void SimpleStorageBucket::read_all(ReadCallback&& callback,
                                   std::function<void()>&& on_done) {
  path p(storage_root);

  try {
    for (auto&& x : fs::directory_iterator(p)) {
      if (x.path().extension() == ".data") {
        std::string key = x.path().stem().string();
        if (strand_map.count(key)) {
          this->read(key, std::move(callback));
        }
        // TODO - call on_done once read_all complete
      }
    }
  } catch (const fs::filesystem_error& ex) {
    // std::cout << ex.what() << '\n';
  }

  return;
}

void SimpleStorageBucket::remove_all() {
  path p(storage_root);

  try {
    for (auto&& x : fs::directory_iterator(p)) {
      if (x.path().extension() == ".data") {
        std::string key = x.path().stem().string();
        if (strand_map.count(key)) {
          strand_map.at(key)->post([=]() { remove(key); });
        }
      }
    }
  } catch (const fs::filesystem_error& ex) {
    // std::cout << ex.what() << '\n';
  }

  return;
}

}  // namespace dsa
