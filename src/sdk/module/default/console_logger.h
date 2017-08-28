#ifndef DSA_SDK_MODULE_CONSOLE_LOGGER_H
#define DSA_SDK_MODULE_CONSOLE_LOGGER_H

#include "../logger.h"

namespace dsa {
class ConsoleLogger : public Logger {
 public:
  void log(const std::string & str) override;
};
}

#endif  // DSA_SDK_MODULE_CONSOLE_LOGGER_H
