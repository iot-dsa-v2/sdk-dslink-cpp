#include "dsa_common.h"

#include "console_logger.h"

#include <iostream>
#include <memory>
#include "util/date_time.h"

namespace dsa {

void ConsoleLogger::write_meta(std::ostream& stream, const char* level) {
  stream << '[' << level << DateTime::get_ts() << "]  ";
}

void ConsoleLogger::log(const string_& str, uint8_t lvl) {
  if ((lvl & filter) || level == Logger::ALL___) {
    std::cout << str;
    std::cout.flush();
  }
}
}
