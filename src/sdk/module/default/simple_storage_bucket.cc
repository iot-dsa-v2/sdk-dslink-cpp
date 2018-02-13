#include "dsa_common.h"
#include "module/logger.h"
#include "simple_storage.h"

#include <boost/asio/strand.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include "util/string.h"
namespace dsa {

namespace fs = boost::filesystem;
using boost::filesystem::path;

SimpleStorageBucket::SimpleStorageBucket(const string_& bucket_name,
                                         boost::asio::io_service* io_service,
                                         const string_& storage_root,
                                         const string_& cwd, bool is_binary)
    : _io_service(io_service),
      _is_binary(is_binary),
      _cwd(cwd),
      _storage_root(storage_root),
      _bucket_name(url_encode(bucket_name)) {

  if (!_cwd.empty()) _full_base_path = _cwd + "/";
  if (!_storage_root.empty()) _full_base_path += _storage_root + "/";
  _full_base_path += bucket_name;

  path p(get_storage_path());
  if (!fs::exists(p)) {
    try {
      if (!fs::create_directories(p)) {
        LOG_FATAL(LOG << p.string() << " storage path cannot be created!");
      }
    } catch (const fs::filesystem_error& ex) {
      LOG_FATAL(LOG << p.string() << " storage path cannot be created!");
    }
  }
}

string_ SimpleStorageBucket::get_storage_path(const string_& key) {
  string_ path;
  if (!_full_base_path.empty()) path = _full_base_path + "/";
  if (!key.empty()) path += key;
  return std::move(path);
}

void SimpleStorageBucket::write(const std::string& key, BytesRef&& content) {
  auto write_file = [=]() {
    path p(get_storage_path(key));

    try {
      auto open_mode = std::ios::out | std::ios::trunc;
      if (_is_binary) open_mode = open_mode | std::ios::binary;
      std::ofstream ofs(p.string().c_str(), open_mode);
      if (ofs) {
        ofs.write(reinterpret_cast<const char*>(content->data()),
                  content->size());
        ofs.close();
      } else {
        // TODO: is fatal?
        LOG_FATAL(LOG << "Unable to open " << key << " file to write");
      }
    } catch (const fs::filesystem_error& ex) {
      // TODO: is fatal?
      LOG_ERROR(Logger::_(), LOG << "Write failed for " << key << " file");
    }
  };

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
  auto read_file = [=]() {
    std::vector<uint8_t> vec{};

    path p(get_storage_path(key));

    try {
      if (fs::exists(p) && is_regular_file(p)) {
        size_t size = fs::file_size(p);

        if (size) {
          std::ios::openmode open_mode = std::ios::in;
          if (_is_binary) open_mode = open_mode | std::ios::binary;
          std::ifstream ifs(p.string().c_str(), open_mode);
          if (ifs) {
            vec.resize(static_cast<size_t>(size));
            ifs.read(reinterpret_cast<char*>(&vec.front()),
                     static_cast<size_t>(size));
            ifs.close();
          } else {
            // TODO: is fatal?
            LOG_FATAL(LOG << "Unable to open " << key << " file to read");
          }
        }
      } else {
        LOG_INFO(Logger::_(), LOG << "there is no file to read " << key);
      }
    } catch (const fs::filesystem_error& ex) {
      // TODO: is fatal?
      LOG_ERROR(Logger::_(), LOG << "Read failed for " << key << " file");
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
      path p(get_storage_path(key));

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
    path p(get_storage_path(key));

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
  path p(get_storage_path());

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
  path p(get_storage_path());

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
