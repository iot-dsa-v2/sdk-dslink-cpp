#ifndef DSA_SDK_MODULE_CONSOLE_LOGGER_H
#define DSA_SDK_MODULE_CONSOLE_LOGGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../logger.h"

namespace dsa {
class ConsoleLogger : public Logger {
 public:
  void write_meta(std::ostream& stream, const char* level) override;
  void log(const string_& str) override;
};
}

#endif  // DSA_SDK_MODULE_CONSOLE_LOGGER_H
