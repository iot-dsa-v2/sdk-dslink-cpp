#include "dsa_common.h"
#include <boost/asio/strand.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include "module/logger.h"
#include "simple_storage.h"
#include "util/string_encode.h"

namespace dsa {

namespace fs = boost::filesystem;
using boost::filesystem::path;

// static std::unique_ptr<SimpleSafeStorageBucket> config_bucket(
//    new SimpleSafeStorageBucket("config", nullptr, ""));
// SimpleSafeStorageBucket &SimpleSafeStorageBucket::get_config_bucket() {
//  return *config_bucket;
//};

void SimpleSafeStorageBucket::write(const string_ &key, BytesRef &&content,
                                    bool is_binary) {
  auto write_file = [=]() {
    path templ = boost::filesystem::unique_path();

    path p(get_storage_path_w(key));

    try {
      auto open_mode = std::ios::out | std::ios::trunc;
      if (is_binary) open_mode = open_mode | std::ios::binary;
      std::ofstream ofs(templ.string(), open_mode);
      if (ofs) {
        ofs.write(reinterpret_cast<const char *>(content->data()),
                  content->size());
        ofs.close();

        boost::filesystem::rename(templ, p);
      } else {
        // TODO: is fatal?
        LOG_FATAL(__FILENAME__,
                  LOG << "Unable to open " << key << " file to write");
      }
    } catch (const fs::filesystem_error &ex) {
      // TODO: is fatal?
      LOG_ERROR(__FILENAME__, LOG << "Write failed for " << key << " file");
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
}  // namespace dsa
