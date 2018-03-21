#include <boost/asio/strand.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include "dsa_common.h"
#include "module/logger.h"
#include "simple_storage.h"
#include "util/string_encode.h"
#include "util/temp_file.h"

namespace dsa {

namespace fs = boost::filesystem;
using boost::filesystem::path;

// static std::unique_ptr<SimpleSafeStorageBucket> config_bucket(
//    new SimpleSafeStorageBucket("config", nullptr, ""));
// SimpleSafeStorageBucket &SimpleSafeStorageBucket::get_config_bucket() {
//  return *config_bucket;
//};

void SimpleSafeStorageBucket::write(const string_ &key, BytesRef &&content) {
  auto write_file = [=]() {
    TempFile tmp;
    path templ = tmp.get();

    path p(get_storage_path(key));

    try {
      std::ofstream ofs(templ.string(),
                        std::ios::out | std::ios::trunc | std::ios::binary);
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
