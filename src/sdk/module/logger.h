#ifndef DSA_SDK_MODULE__LOGGER_H
#define DSA_SDK_MODULE__LOGGER_H

#include <sstream>

namespace dsa {

class Logger {
 public:
  enum : uint8_t {
    DEBUG = 0,
    INFO = 0x10,
    WARNING = 0x20,
    ERROR = 0x30,
    CRITICAL = 0x40,
    NONE = 0xFF,
  };

 public:
  uint8_t level = 0xF0;

  virtual void log(const std::string& str) = 0;
  virtual ~Logger() = default;
};
}

#define LOG_CRITICAL(logger, stream)        \
  if ((logger).level <= Logger::CRITICAL) { \
    std::stringstream _;                    \
    stream << std::endl;                    \
    (logger).log(_.str());                  \
  }
#define LOG_ERROR(logger, stream)        \
  if ((logger).level <= Logger::ERROR) { \
    std::stringstream _;                 \
    stream << std::endl;                 \
    (logger).log(_.str());               \
  }
#define LOG_WARNING(logger, stream)        \
  if ((logger).level <= Logger::WARNING) { \
    std::stringstream _;                   \
    stream << std::endl;                   \
    (logger).log(_.str());                 \
  }
#define LOG_INFO(logger, stream)        \
  if ((logger).level <= Logger::INFO) { \
    std::stringstream _;                \
    stream << std::endl;                \
    (logger).log(_.str());              \
  }
#define LOG_DEBUG(logger, stream)        \
  if ((logger).level <= Logger::DEBUG) { \
    std::stringstream _;                 \
    stream << std::endl;                 \
    (logger).log(_.str());               \
  }

#endif  // DSA_SDK_MODULE__LOGGER_H
