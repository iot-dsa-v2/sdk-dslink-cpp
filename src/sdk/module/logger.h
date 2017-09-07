#ifndef DSA_SDK_MODULE__LOGGER_H
#define DSA_SDK_MODULE__LOGGER_H

#include <sstream>

namespace dsa {

class Logger {
 public:
  enum : uint8_t {
    TRACE = 0x10,
    DEBUG = 0x20,
    INFO = 0x30,
    WARN = 0x40,
    ERROR = 0x50,
    FATAL = 0x60,
    NONE = 0xFF,
  };

 public:
  uint8_t level = 0;

  virtual void write_meta(std::ostream& stream, const char* level) = 0;
  virtual void log(const std::string& str) = 0;
  virtual ~Logger() = default;
};
}

#define DSA_LOG(LEVEL, logger, stream_exp) \
  if ((logger).level <= Logger::LEVEL) {   \
    std::stringstream LOG;                 \
    (logger).write_meta(LOG, "LEVEL");     \
    stream_exp;                            \
    LOG << std::endl;                      \
    (logger).log(LOG.str());               \
  }

#define LOG_ERROR(logger, stream_exp) DSA_LOG(ERROR, logger, stream_exp)

#define LOG_WARN(logger, stream_exp) DSA_LOG(WARN, logger, stream_exp)

#define LOG_INFO(logger, stream_exp) DSA_LOG(INFO, logger, stream_exp)

#define LOG_DEBUG(logger, stream_exp) DSA_LOG(DEBUG, logger, stream_exp)

#define LOG_TRACE(logger, stream_exp) DSA_LOG(TRACE, logger, stream_exp)

#define LOG_FATAL(logger, stream_exp) \
  {                                          \
    std::stringstream LOG;                   \
    (logger).write_meta(LOG, "FATAL");       \
    stream_exp;                              \
    LOG << std::endl;                        \
    if ((logger).level <= Logger::FATAL) {   \
      (logger).log(LOG.str());               \
    }                                        \
    throw std::runtime_error(LOG.str());      \
  }

#endif  // DSA_SDK_MODULE__LOGGER_H
