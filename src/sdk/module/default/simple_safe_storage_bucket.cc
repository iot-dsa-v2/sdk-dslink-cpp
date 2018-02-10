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
static char templ[] = "/tmp/fileXXXXXX";
#endif

inline void SimpleSafeStorageBucket::write_file(const std::string& key,
                                     BytesRef content) {
#if (defined (_WIN32) || defined (_WIN64))
  std::string templ = tmpnam(nullptr)
#else
  mkstemp(templ);
#endif
  path p(storage_root);

  p /= (key);

  try {
    std::ofstream ofs(templ, std::ios::out | std::ios::trunc);
    if (ofs) {
      ofs.write(reinterpret_cast<const char *>(content->data()),
                content->size());

      boost::filesystem::rename(templ, p);
    } else {
      // TODO - error handling
    }
  } catch (const fs::filesystem_error &ex) {
    // TODO - error handling
  }
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