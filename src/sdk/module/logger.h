#ifndef DSA_SDK_MODULE__LOGGER_H
#define DSA_SDK_MODULE__LOGGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <sstream>

namespace dsa {

class Logger {
 public:
  enum : uint8_t {
    TRACE_ = 0x10,
    DEBUG_ = 0x20,
    INFO_ = 0x30,
    WARN_ = 0x40,
    ERROR_ = 0x50,
    FATAL_ = 0x60,
    NONE_ = 0xFF,
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
    LOG << std::endl;                      \
    (logger).write_meta(LOG, #LEVEL);      \
    stream_exp;                            \
    (logger).log(LOG.str());               \
  }

#define LOG_ERROR(logger, stream_exp) DSA_LOG(ERROR_, logger, stream_exp)

#define LOG_WARN(logger, stream_exp) DSA_LOG(_WARN_, logger, stream_exp)

#define LOG_INFO(logger, stream_exp) DSA_LOG(INFO_, logger, stream_exp)

#define LOG_DEBUG(logger, stream_exp) DSA_LOG(DEBUG_, logger, stream_exp)

#define LOG_TRACE(logger, stream_exp) DSA_LOG(TRACE_, logger, stream_exp)

#define LOG_FATAL(logger, stream_exp)       \
  {                                         \
    std::stringstream LOG;                  \
    LOG << std::endl;                       \
    (logger).write_meta(LOG, "FATAL_");     \
    stream_exp;                             \
    if ((logger).level <= Logger::FATAL_) { \
      (logger).log(LOG.str());              \
    }                                       \
    throw std::runtime_error(LOG.str());    \
  }

#endif  // DSA_SDK_MODULE__LOGGER_H
