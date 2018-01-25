#include "dsa_common.h"
#include "simple_storage.h"

#include <boost/asio/strand.hpp>
#include <boost/filesystem.hpp>
#include <fstream>

namespace dsa {

namespace fs = boost::filesystem;
using boost::filesystem::path;

#if (defined (_WIN32) || defined (_WIN64))
static std::string storage_root = "C:\\temp\\";
#else
static std::string storage_root = "";
#endif

inline void SimpleSafeStorageBucket::write_file(const std::string& key,
                                     BytesRef content) {
  path p(storage_root);
  path p_temp = std::tmpnam(nullptr);

  p /= (key);

  try {
    std::ofstream ofs(p_temp.string().c_str(), std::ios::out | std::ios::trunc);
    if (ofs) {
      ofs.write(reinterpret_cast<const char *>(content->data()),
                content->size());
      boost::filesystem::rename(p_temp, p);
    } else {
      // TODO - error handling
    }
  } catch (const fs::filesystem_error &ex) {
    // TODO - error handling
  }

  return;
}

void SimpleSafeStorageBucket::write(const std::string &key, BytesRef &&content) {
  if (_io_service != nullptr) {
    if (!strand_map.count(key)) {
      boost::asio::io_service::strand* strand =
          new boost::asio::io_service::strand(*_io_service);
      strand_map.insert(StrandPair(key, strand));
    }

    strand_map.at(key)->post([=]() {
      write_file(key, content);
      return;
    });
    return;
  } else {
    write_file(key, std::move(content));
  }
}
}