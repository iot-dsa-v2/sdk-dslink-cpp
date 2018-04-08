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

// static std::unique_ptr<SimpleSafeStorageBucket> config_bucket(
//    new SimpleSafeStorageBucket("config", nullptr, ""));
// SimpleSafeStorageBucket &SimpleSafeStorageBucket::get_config_bucket() {
//  return *config_bucket;
//};

static void _write(SimpleSafeStorageBucket *bucket, const string_ &key,
                   BytesRef &&content) {
  TempFile tmp;
  fs::path templ = tmp.get();

  const fs::path p = bucket->get_storage_path(key);

  try {
    std::ofstream ofs(templ.string(),
                      std::ios::out | std::ios::trunc | std::ios::binary);
    if (ofs) {
      ofs.write(reinterpret_cast<const char *>(content->data()),
                content->size());
      ofs.close();
      boost::system::error_code err;
      fs::rename(templ, p, err);
      if (err.value() == EXDEV) {
        // when rename is not allowed
        fs::copy_file(templ, p, fs::copy_option::overwrite_if_exists, err);
        if (!err) {
          fs::remove(templ);
        } else {
          LOG_ERROR(__FILENAME__,
                    LOG << "Failed to rename or copy the file from " << templ
                        << " to " << p);
        }
      }
    } else {
      LOG_ERROR(__FILENAME__,
                LOG << "Unable to open " << templ << " file to write");
    }
  } catch (const fs::filesystem_error &ex) {
    LOG_ERROR(__FILENAME__, LOG << "Write failed for " << key << " file");
  }
}

void SimpleSafeStorageBucket::write(const string_ &key, BytesRef &&content) {
  if (_io_service != nullptr) {
    if (!strand_map.count(key)) {
      boost::asio::io_service::strand *strand =
          new boost::asio::io_service::strand(*_io_service);
      strand_map.insert(StrandPair(key, strand));
    }

    strand_map.at(key)->post([
      keep_ptr = shared_from_this(), key, content = std::move(content)
    ]() mutable {
      _write(static_cast<SimpleSafeStorageBucket *>(keep_ptr.get()), key,
             std::move(content));
      return;
    });
    return;
  } else {
    _write(this, key, std::move(content));
  }
}
}  // namespace dsa
