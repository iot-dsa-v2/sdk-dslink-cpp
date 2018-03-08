#include "dsa_common.h"
#include "module/logger.h"
#include "simple_storage.h"

#include <boost/asio/strand.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <list>
#include "util/misc.h"
#include "util/string_encode.h"
#include <string>
#include <locale>
#include <codecvt>
#include <iomanip>
#include <vector>
namespace dsa {

namespace fs = boost::filesystem;
using boost::filesystem::path;

SimpleStorageBucket::SimpleStorageBucket(const string_& bucket_name,
                                         boost::asio::io_service* io_service,
                                         const string_& storage_root,
                                         const string_& cwd)
    : _io_service(io_service),
      _storage_root(storage_root),
      _bucket_name(url_encode_file_name(bucket_name)) {
  _cwd = cwd;
  if (cwd.empty()) _cwd = get_current_working_dir();
  if (!_cwd.empty()) _full_base_path = _cwd + "/";
  if (!_storage_root.empty()) _full_base_path += _storage_root + "/";
  _full_base_path += bucket_name;

  path p(get_storage_path());
  if (!fs::exists(p)) {
    try {
      if (!fs::create_directories(p)) {
        LOG_FATAL(__FILENAME__,
                  LOG << p.string() << " storage path cannot be created!");
      }
    } catch (const fs::filesystem_error& ex) {
      LOG_FATAL(__FILENAME__,
                LOG << p.string() << " storage path cannot be created!");
    }
  }
}
#if defined(_WIN32) || defined(_WIN64)
std::wstring SimpleStorageBucket::get_storage_path(const string_& key) {
  string_ path;
  if (!_full_base_path.empty()) path = _full_base_path + "/";
  if (!key.empty()) path += url_encode_file_name(key);
  return std::move(
      std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}
          .from_bytes(path));
}
#else
string_ SimpleStorageBucket::get_storage_path(const string_& key) {
  string_ path;
  if (!_full_base_path.empty()) path = _full_base_path + "/";
  if (!key.empty()) path += url_encode_file_name(key);
  return std::move(path);
}
#endif

bool SimpleStorageBucket::is_empty() {
  path p(get_storage_path());
  if (!boost::filesystem::is_directory(p)) return false;

  boost::filesystem::directory_iterator end_it;
  boost::filesystem::directory_iterator it(p);
  if (it == end_it)
    return true;
  else
    return false;
}

bool SimpleStorageBucket::exists(const string_& key) {
  path p(get_storage_path(key));
  return (fs::exists(p) && is_regular_file(p));
}

void SimpleStorageBucket::write(const string_& key, BytesRef&& content,
                                bool is_binary) {
  auto write_file = [=, content = std::move(content)]() {
    path p(get_storage_path(key));

    try {
      auto open_mode = std::ios::out | std::ios::trunc;
      if (is_binary) open_mode = open_mode | std::ios::binary;
      fs::ofstream ofs(p, open_mode);
      if (ofs) {
        ofs.write(reinterpret_cast<const char*>(content->data()),
                  content->size());
        ofs.close();
      } else {
        // TODO: is fatal?
        LOG_FATAL(__FILENAME__,
                  LOG << "Unable to open " << key << " file to write");
      }
    } catch (const fs::filesystem_error& ex) {
      // TODO: is fatal?
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

void SimpleStorageBucket::read(const string_& key, ReadCallback&& callback,
                               bool is_binary) {
  auto read_file = [=, callback = std::move(callback)]() {
    BucketReadStatus status = BucketReadStatus::OK;
    std::vector<uint8_t> vec{};

    path p(get_storage_path(key));

    try {
      if (fs::exists(p) && is_regular_file(p)) {
        size_t size = fs::file_size(p);

        if (size) {
          std::ios::openmode open_mode = std::ios::in;
          if (is_binary) open_mode = open_mode | std::ios::binary;
          fs::ifstream ifs(p, open_mode);
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

    boost::asio::io_service::strand* owner_strand = get_owner_strand();

    if (owner_strand != nullptr)
      owner_strand->post([
        callback = std::move(callback), decode_key = std::move(url_decode(key)),
        vec = std::move(vec), status = std::move(status)
      ]() {
        callback(std::move(decode_key), std::move(vec), std::move(status));
      });
    else
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
    if (!strand_map.count(key)) {
      boost::asio::io_service::strand* strand =
          new boost::asio::io_service::strand(*_io_service);
      strand_map.insert(StrandPair(key, strand));
    }

    strand_map.at(key)->post([=]() {
      path p(get_storage_path(key));

      try {
        if (fs::exists(p) && is_regular_file(p)) {
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
  path p(get_storage_path());
  std::list<std::wstring> key_list;
  try {
    for (auto&& x : fs::directory_iterator(p)) {
      key_list.push_back(x.path().stem().wstring());
    }
    key_list.sort();
    for (auto&& key : key_list) {
      auto cb = callback;
      this->read(
          url_decode(
              std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(key)),
          std::move(cb));
    }
    if (on_done != nullptr) on_done();
  } catch (const fs::filesystem_error& ex) {
    // std::cout << ex.what() << '\n';
  }

  return;
}

void SimpleStorageBucket::remove_all() {
  path p(get_storage_path());
  std::list<std::wstring> key_list;
  try {
    for (auto&& x : fs::directory_iterator(p)) {
      key_list.push_back(x.path().stem().wstring());
    }
    for (auto&& key : key_list) {
      remove(url_decode(
          std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(key)));
    }
  } catch (const fs::filesystem_error& ex) {
    // std::cout << ex.what() << '\n';
  }

  return;
}

}  // namespace dsa
