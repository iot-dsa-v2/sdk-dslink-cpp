#include "dsa_common.h"
#include "misc.h"
#include <boost/filesystem.hpp>
#include <codecvt>
#include <iomanip>
#include <iostream>
#include <locale>
#include <string>
#include <vector>

namespace dsa {
string_ get_current_working_dir() {
  return std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(
      boost::filesystem::current_path().wstring());
}
}  // namespace dsa
