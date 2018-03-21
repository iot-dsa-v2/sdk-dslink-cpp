#ifndef DSA_SDK_TEMP_FILE_H
#define DSA_SDK_TEMP_FILE_H
#include <boost/filesystem.hpp>
#include "dsa_common.h"

namespace dsa {
class TempFile {
  boost::filesystem::path tmp_file_prefix;
  std::atomic<int> counter{0};

 public:
  TempFile();
  void init(dsa::string_ name);
  boost::filesystem::path get();
};
}
#endif  // DSA_SDK_TEMP_FILE_H
