#ifndef DSA_SDK_MODULE_CONSOLE_LOGGER_H
#define DSA_SDK_MODULE_CONSOLE_LOGGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../logger.h"

namespace dsa {
class ConsoleLogger : public Logger {
 public:
  uint8_t filter = 0xFF;

  void write_meta(std::ostream& stream, const char* level,
                  const string_& log_name) override;
  void log(const string_& str, uint8_t level) override;
};
}

#endif  // DSA_SDK_MODULE_CONSOLE_LOGGER_H
