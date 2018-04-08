#include "dsa_common.h"
#include "module/logger.h"
#include "simple_storage.h"

#include <boost/asio/strand.hpp>
#include <boost/filesystem.hpp>
#include <codecvt>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <locale>
#include <string>
#include <vector>
#include "util/misc.h"
#include "util/string_encode.h"
namespace dsa {

namespace fs = boost::filesystem;

SimpleStorageBucket::SimpleStorageBucket(const string_& bucket_name,
                                         boost::asio::io_service* io_service,
                                         const string_& storage_root,
                                         const string_& cwd)
    : _io_service(io_service),
      _storage_root(storage_root),
      _bucket_name(bucket_name) {
  _cwd = cwd;
  if (cwd.empty()) _cwd = get_current_working_dir();
  if (!_cwd.empty()) _full_base_str = _cwd + "/";
  if (!_storage_root.empty()) _full_base_str += _storage_root + "/";
  _full_base_str += url_encode_file_name(_bucket_name);
  _full_base_path = utf8_str_to_path(_full_base_str);
  if (!fs::exists(_full_base_path)) {
    try {
      if (!fs::create_directories(_full_base_path)) {
        LOG_FATAL(__FILENAME__,
                  LOG << _full_base_str << " storage path cannot be created!");
      }
    } catch (const fs::filesystem_error& ex) {
      LOG_FATAL(__FILENAME__,
                LOG << _full_base_str << " storage path cannot be created!");
    }
  }
}

fs::path SimpleStorageBucket::get_storage_path(const string_& key) {
  return utf8_str_to_path(_full_base_str + "/" + url_encode_file_name(key));
}

bool SimpleStorageBucket::is_empty() {
  if (!boost::filesystem::is_directory(_full_base_path)) return true;

  boost::filesystem::directory_iterator end_it;
  boost::filesystem::directory_iterator it(_full_base_path);
  if (it == end_it)
    return true;
  else
    return false;
}

bool SimpleStorageBucket::exists(const string_& key) {
  fs::path p = get_storage_path(key);
  return (fs::exists(p) && is_regular_file(p));
}

void SimpleStorageBucket::write(const string_& key, BytesRef&& content) {
  auto write_file = [ =, content = std::move(content) ]() {
    fs::path p = get_storage_path(key);

    try {
      fs::ofstream ofs(p, std::ios::out | std::ios::trunc | std::ios::binary);
      if (ofs) {
        ofs.write(reinterpret_cast<const char*>(content->data()),
                  content->size());
        ofs.close();
      } else {
        LOG_ERROR(__FILENAME__,
                  LOG << "Unable to open " << key << " file to write");
      }
    } catch (const fs::filesystem_error& ex) {
      LOG_ERROR(__FILENAME__, LOG << "Write failed for " << key << " file");
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

void SimpleStorageBucket::read(const string_& key, ReadCallback&& callback) {
  auto read_file = [ =, callback = std::move(callback) ]() {
    BucketReadStatus status = BucketReadStatus::OK;
    std::vector<uint8_t> vec{};

    fs::path p = get_storage_path(key);

    try {
      if (fs::exists(p) && is_regular_file(p)) {
        size_t size = fs::file_size(p);

        if (size) {
          fs::ifstream ifs(p, std::ios::in | std::ios::binary);
          if (ifs) {
            vec.resize(static_cast<size_t>(size));
            ifs.read(reinterpret_cast<char*>(&vec.front()),
                     static_cast<size_t>(size));
            ifs.close();
          } else {
            LOG_ERROR(__FILENAME__,
                      LOG << "Unable to open " << key << " file to read");
            status = BucketReadStatus::FILE_OPEN_ERROR;
          }
        }
      } else {
        LOG_FINE(__FILENAME__, LOG << "there is no file to read " << key);
        status = BucketReadStatus::NOT_EXIST;
      }
    } catch (const fs::filesystem_error& ex) {
      LOG_ERROR(__FILENAME__, LOG << "Read failed for " << key << " file");
      status = BucketReadStatus::READ_FAILED;
    }
    callback(std::move(url_decode(key)), std::move(vec), std::move(status));
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

void SimpleStorageBucket::remove(const string_& key) {
  if (_io_service != nullptr) {
    {
      std::lock_guard<std::mutex> lock(remove_mutex);
      if (!strand_map.count(key)) {
        boost::asio::io_service::strand* strand =
            new boost::asio::io_service::strand(*_io_service);
        strand_map.insert(StrandPair(key, strand));
      }

      strand_map.at(key)->post([=]() {
        fs::path p =
            utf8_str_to_path(_full_base_str + "/" + url_encode_file_name(key));

        try {
          if (fs::exists(p) && is_regular_file(p)) {
            fs::remove(p);
            std::lock_guard<std::mutex> lock(remove_mutex);
            delete strand_map.at(key);
            strand_map.erase(
                key);  // erase the element from the map as well, if
            // it is removed successfully
          }
        } catch (const fs::filesystem_error& ex) {
          ;
        }

        return;
      });
    }
    return;
  } else {
    fs::path p = get_storage_path(key);

    try {
      if (fs::exists(p) && is_regular_file(p)) {
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
  shared_ptr_<ReadCbTrack> read_cb_track = make_shared_<ReadCbTrack>();
  read_cb_track->num_needed = 0;

  std::list<string_> key_list;
  try {
    for (auto&& x : fs::directory_iterator(_full_base_path)) {
      key_list.push_back(path_to_utf8_str(x.path().filename()));
      read_cb_track->num_needed++;
    }
    key_list.sort();
    for (auto&& key : key_list) {
      auto cb = callback;
      auto on_done_cb = on_done;
      this->read(url_decode(key),
                 [ =, cb = std::move(cb), on_done_cb = std::move(on_done_cb) ](
                     const string_& key, std::vector<uint8_t> data,
                     BucketReadStatus read_status) {

                   // TODO: consider not lock callback, but it that case the
                   // callback
                   // is not going to be thread safe
                   // thread safety should be provided in the callback
                   std::lock_guard<std::mutex> lock(read_cb_track->read_mutex);
                   cb(std::move(key), std::move(data), read_status);
                   read_cb_track->num_needed--;
                   if (read_cb_track->num_needed == 0) {
                     if (on_done_cb != nullptr) on_done_cb();
                   }

                 });
    }
  } catch (const fs::filesystem_error& ex) {
    // std::cout << ex.what() << '\n';
  }

  return;
}

void SimpleStorageBucket::remove_all() {
  std::list<string_> key_list;
  try {
    for (auto&& x : fs::directory_iterator(_full_base_path)) {
      key_list.push_back(path_to_utf8_str(x.path().filename()));
    }
    for (auto&& key : key_list) {
      remove(url_decode(key));
    }
  } catch (const fs::filesystem_error& ex) {
    // std::cout << ex.what() << '\n';
  }

  return;
}
void SimpleStorageBucket::destroy_bucket() {
  std::lock_guard<std::mutex> lock(remove_mutex);
  for (auto& strand : strand_map) {
    delete strand_map.at(strand.first);
  }
  strand_map.clear();
}

}  // namespace dsa
