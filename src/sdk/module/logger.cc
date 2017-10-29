#include "dsa_common.h"

#include "logger.h"

#include <iostream>
#include <memory>
#include "default/console_logger.h"

namespace dsa {

static std::unique_ptr<Logger> default_logger(new ConsoleLogger());
Logger& Logger::_() { return *default_logger; }
void Logger::set_default(Logger* logger) { default_logger.reset(logger); }
}
