#ifndef DSA_SDK_TEMP_FILE_H
#define DSA_SDK_TEMP_FILE_H
#include <boost/filesystem.hpp>
#include "dsa_common.h"

namespace dsa {
class TempFile {
  static boost::filesystem::path tmp_file_prefix;
  static std::atomic<int> counter;

 public:
  static void init(dsa::string_ name);
  static boost::filesystem::path get();
};
}  // namespace dsa
#endif  // DSA_SDK_TEMP_FILE_H
