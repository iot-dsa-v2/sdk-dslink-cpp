#ifndef DSA_SDK_MODULE_LOGGER_H_
#define DSA_SDK_MODULE_LOGGER_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <sstream>

namespace dsa {

class Logger {
 public:
  enum : uint8_t {
    ALL___ = 0x00,
    TRACE_ = 0x10,
    DEBUG_ = 0x20,
    INFO__ = 0x30,
    WARN__ = 0x40,
    ERROR_ = 0x50,
    SYS___ = 0x60,
    FATAL_ = 0x70,
    NONE__ = 0xFF,
  };

  // default logger, implemented in console_logger.cc
  static Logger& _();
  // the new logger will be maintained by unique pointer and no need for delete
  static void set_default(Logger* logger);

  static uint8_t parse(const std::string& log);

 public:
  uint8_t level = 0;

  virtual void write_meta(std::ostream& stream, const char* level) = 0;
  virtual void log(const string_& str) = 0;
  virtual ~Logger() = default;
};
}

#define DSA_LOG(LEVEL, logger, stream_exp) \
  if ((logger).level <= Logger::LEVEL) {   \
    std::stringstream LOG;                 \
    LOG << std::endl;                      \
    (logger).write_meta(LOG, #LEVEL);      \
    stream_exp;                            \
    (logger).log(LOG.str());               \
  }

#define LOG_ERROR(logger, stream_exp) DSA_LOG(ERROR_, logger, stream_exp)

#define LOG_WARN(logger, stream_exp) DSA_LOG(WARN__, logger, stream_exp)

#define LOG_INFO(logger, stream_exp) DSA_LOG(INFO__, logger, stream_exp)

#define LOG_SYSTEM(logger, stream_exp) DSA_LOG(SYS___, logger, stream_exp)

#define LOG_DEBUG(logger, stream_exp) DSA_LOG(DEBUG_, logger, stream_exp)

#define LOG_TRACE(logger, stream_exp) DSA_LOG(TRACE_, logger, stream_exp)

#define LOG_FATAL(stream_exp)               \
  {                                         \
    std::stringstream LOG;                  \
    LOG << std::endl;                       \
    dsa::Logger& logger = Logger::_();      \
    if ((logger).level <= Logger::FATAL_) { \
      (logger).write_meta(LOG, "FATAL_");   \
      stream_exp;                           \
      (logger).log(LOG.str());              \
    }                                       \
    exit(1);                                \
  }

#endif  // DSA_SDK_MODULE_LOGGER_H_
