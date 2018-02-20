#include "dsa_common.h"

#include "logger.h"

#include <iostream>
#include <memory>
#include "default/console_logger.h"

namespace dsa {

static shared_ptr_<Logger> default_logger(make_shared_<ConsoleLogger>());
Logger& Logger::_() { return *default_logger; }
void Logger::set_default(shared_ptr_<Logger> logger) {
  default_logger = logger;
}

uint8_t Logger::parse(const std::string& log) {
  if (log == "all") {
    return Logger::ALL___;
  } else if (log == "trace") {
    return Logger::TRACE_;
  } else if (log == "debug") {
    return Logger::DEBUG_;
  } else if (log == "warn") {
    return Logger::WARN__;
  } else if (log == "info") {
    return Logger::INFO__;
  } else if (log == "error") {
    return Logger::ERROR_;
  } else if (log == "admin") {
    return Logger::ADMIN_;
  } else if (log == "fatal") {
    return Logger::FATAL_;
  } else if (log == "none") {
    return Logger::NONE__;
  } else {  // default
    return Logger::FINE__;
  }
}

void Logger::destroy_impl() { level = NONE__; }
}
