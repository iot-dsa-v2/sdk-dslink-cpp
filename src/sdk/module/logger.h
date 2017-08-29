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
  uint8_t level = 0;

  virtual void write_meta(std::ostream& stream, const char* level) = 0;
  virtual void log(const std::string& str) = 0;
  virtual ~Logger() = default;
};
}

#define LOG_CRITICAL(logger, stream_exp)    \
  if ((logger).level <= Logger::CRITICAL) { \
    std::stringstream LOG;                  \
    (logger).write_meta(LOG, "CRITICAL");   \
    stream_exp;                             \
    LOG << std::endl;                       \
    (logger).log(LOG.str());                \
  }
#define LOG_ERROR(logger, stream_exp)    \
  if ((logger).level <= Logger::ERROR) { \
    std::stringstream LOG;               \
    (logger).write_meta(LOG, "ERROR");   \
    stream_exp;                          \
    LOG << std::endl;                    \
    (logger).log(LOG.str());             \
  }
#define LOG_WARNING(logger, stream_exp)    \
  if ((logger).level <= Logger::WARNING) { \
    std::stringstream LOG;                 \
    (logger).write_meta(LOG, "WARNING");   \
    stream_exp;                            \
    LOG << std::endl;                      \
    (logger).log(LOG.str());               \
  }
#define LOG_INFO(logger, stream_exp)    \
  if ((logger).level <= Logger::INFO) { \
    std::stringstream LOG;              \
    (logger).write_meta(LOG, "INFO");   \
    stream_exp;                         \
    LOG << std::endl;                   \
    (logger).log(LOG.str());            \
  }
#define LOG_DEBUG(logger, stream_exp)    \
  if ((logger).level <= Logger::DEBUG) { \
    std::stringstream LOG;               \
    (logger).write_meta(LOG, "DEBUG");   \
    stream_exp;                          \
    LOG << std::endl;                    \
    (logger).log(LOG.str());             \
  }

#endif  // DSA_SDK_MODULE__LOGGER_H
