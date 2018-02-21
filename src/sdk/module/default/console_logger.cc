#include "dsa_common.h"

#include "console_logger.h"

#include <iostream>
#include <memory>
#include "util/date_time.h"

namespace dsa {

void ConsoleLogger::write_meta(std::ostream& stream, const char* level,
                               const string_& log_name) {
  const string_& ts = DateTime::get_ts();
  // use substrings of the timestamp
  stream << level << string_(&ts[0], &ts[10]) << " "
         << string_(&ts[11], &ts[23]) << " [" << log_name << "]  ";
}

void ConsoleLogger::log(const string_& str, uint8_t lvl) {
  if ((lvl & filter) || level == Logger::ALL___) {
    std::cout << str;
    std::cout.flush();
  }
}
}
