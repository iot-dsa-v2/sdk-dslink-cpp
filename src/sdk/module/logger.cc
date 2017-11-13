#include "dsa_common.h"

#include "logger.h"

#include <iostream>
#include <memory>
#include "default/console_logger.h"

namespace dsa {

static std::unique_ptr<Logger> default_logger(new ConsoleLogger());
Logger& Logger::_() { return *default_logger; }
void Logger::set_default(Logger* logger) { default_logger.reset(logger); }

uint8_t Logger::parse(const std::string& log) {
  if (log == "all") {
    return Logger::ALL___;
  } else if (log == "trace") {
    return Logger::TRACE_;
  } else if (log == "debug") {
    return Logger::DEBUG_;
  } else if (log == "warn") {
    return Logger::WARN__;
  } else if (log == "error") {
    return Logger::ERROR_;
  } else if (log == "fatal") {
    return Logger::FATAL_;
  } else if (log == "none") {
    return Logger::NONE__;
  } else {  // default
    return Logger::INFO__;
  }
}
}
