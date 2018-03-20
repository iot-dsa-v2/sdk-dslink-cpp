#include "temp_file.h"
#include <boost/filesystem.hpp>
#include "dsa_common.h"
#include "module/logger.h"
#include "path.h"

namespace fs = boost::filesystem;

namespace dsa {
TempFile::TempFile() {}

void TempFile::init(dsa::string_ name) {
  boost::system::error_code ec;
  fs::path tmp_dir_path(fs::temp_directory_path().string() + "/" + name);

  if (fs::exists(tmp_dir_path)) {
    for (auto &&it : fs::directory_iterator(tmp_dir_path)) {
      std::time_t ftime = fs::last_write_time(it.path());
      std::time_t now = time(0);
      if ((time(0) - fs::last_write_time(it.path())) > 3600) {
        fs::remove_all(it.path(), ec);
        if (ec)
          LOG_FATAL(__FILENAME__,
                    LOG << "Unable to delete " << it.path().string());
      }
    }

  } else {
    auto tmp_dir = fs::create_directory(tmp_dir_path);
    if (!tmp_dir)
      LOG_FATAL(__FILENAME__,
                LOG << "Unable to create " << tmp_dir_path.string()
                    << " temp folder");
  }
  tmp_file_prefix = fs::temp_directory_path().string() + "/" + name + "/" +
                    boost::filesystem::unique_path().string();
}

fs::path TempFile::get() {
  dsa::string_ unique_path;
  if (!tmp_file_prefix.empty()) {
    unique_path = tmp_file_prefix.string() + "_" + std::to_string(counter);
    counter++;
  } else {
    unique_path = fs::temp_directory_path().string() + "/" +
                  boost::filesystem::unique_path().string();
  }

  auto tmp_file = std::ofstream(unique_path.c_str());

  if (tmp_file.bad())
    LOG_FATAL(__FILENAME__,
              LOG << "Unable to create " << unique_path << " log file");

  return fs::path(unique_path);
}
}