#include "dsa_common.h"

#include "console_logger.h"

#include <iostream>
#include <memory>
#include "util/date_time.h"

namespace dsa {

static std::unique_ptr<Logger> default_logger(new ConsoleLogger());
Logger& Logger::_() { return *default_logger; }
void Logger::set_default(Logger* logger) { default_logger.reset(logger); }

void ConsoleLogger::write_meta(std::ostream& stream, const char* level) {
  stream << '[' << level << ' ' << DateTime::get_ts() << "]  ";
}
void ConsoleLogger::log(const std::string& str) { std::cout << str; }
}
