#include <boost/asio/strand.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include "dsa_common.h"
#include "module/logger.h"
#include "simple_storage.h"

namespace dsa {

namespace fs = boost::filesystem;
using boost::filesystem::path;

#if (defined(_WIN32) || defined(_WIN64))
static std::string storage_root = "C:\\temp\\";
#else
static std::string storage_root = "";
static char templ[] = "/tmp/fileXXXXXX";
#endif

void SimpleSafeStorageBucket::write(const std::string &key,
                                    BytesRef &&content) {
  auto write_file = [=]() {
#if (defined(_WIN32) || defined(_WIN64))
    std::string templ = tmpnam(nullptr);
#else
    mkstemp(templ);
#endif
    path p(storage_root);

    p /= (key);

    try {
      auto open_mode = std::ios::out | std::ios::trunc;
      if (_is_binary) open_mode = open_mode | std::ios::binary;
      std::ofstream ofs(templ, open_mode);
      if (ofs) {
        ofs.write(reinterpret_cast<const char *>(content->data()),
                  content->size());
        ofs.close();

        boost::filesystem::rename(templ, p);
      } else {
        // TODO: is fatal?
        LOG_FATAL(LOG << "Unable to open " << key << " file to write");
      }
    } catch (const fs::filesystem_error &ex) {
      // TODO: is fatal?
      LOG_ERROR(Logger::_(), LOG << "Write failed for " << key << " file");
    }
  };

  if (_io_service != nullptr) {
    if (!strand_map.count(key)) {
      boost::asio::io_service::strand *strand =
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
}