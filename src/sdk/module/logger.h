#ifndef DSA_SDK_MODULE_LOGGER_H
#define DSA_SDK_MODULE_LOGGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <sstream>
#include "util/enable_shared.h"

namespace dsa {

class Logger : public SharedDestroyable<Logger> {
 public:
  enum : uint8_t {
    ALL___ = 0x00,
    TRACE_ = 0x01,
    DEBUG_ = 0x02,
    INFO__ = 0x04,
    WARN__ = 0x08,
    SYS___ = 0x10,
    ERROR_ = 0x20,
    ADMIN_ = 0x40,
    FATAL_ = 0x80,
    NONE__ = 0xFF,
  };

  // default logger, implemented in console_logger.cc
  static Logger& _();
  // the new logger will be maintained by unique pointer and no need for delete
  static void set_default(shared_ptr_<Logger> logger);

  static uint8_t parse(const std::string& log);

 public:
  uint8_t level = 0;

  virtual void write_meta(std::ostream& stream, const char* level) = 0;
  virtual void log(const string_& str, uint8_t level) = 0;
  virtual ~Logger() = default;

 protected:
  void destroy_impl() override;
};
}

#define DSA_LOG(LEVEL, LEVEL_NAME, logger, stream_exp) \
  if ((logger).level <= Logger::LEVEL) {               \
    std::stringstream LOG;                             \
    LOG << std::endl;                                  \
    (logger).write_meta(LOG, LEVEL_NAME);              \
    stream_exp;                                        \
    (logger).log(LOG.str(), Logger::LEVEL);            \
  }

#define LOG_ERROR(logger, stream_exp) \
  DSA_LOG(ERROR_, "Error ", logger, stream_exp)

#define LOG_WARN(logger, stream_exp) \
  DSA_LOG(WARN__, "Warn  ", logger, stream_exp)

#define LOG_INFO(logger, stream_exp) \
  DSA_LOG(INFO__, "Info  ", logger, stream_exp)

#define LOG_SYSTEM(logger, stream_exp) \
  DSA_LOG(SYS___, "Sys   ", logger, stream_exp)

#define LOG_ADMIN(logger, stream_exp) \
  DSA_LOG(ADMIN___, "Admin ", logger, stream_exp)

#define LOG_DEBUG(logger, stream_exp) \
  DSA_LOG(DEBUG_, "Debug ", logger, stream_exp)

#define LOG_TRACE(logger, stream_exp) \
  DSA_LOG(TRACE_, "Trace ", logger, stream_exp)

#define LOG_FATAL(stream_exp)                  \
  {                                            \
    std::stringstream LOG;                     \
    LOG << std::endl;                          \
    dsa::Logger& logger = Logger::_();         \
    if ((logger).level <= Logger::FATAL_) {    \
      (logger).write_meta(LOG, "Fatal ");      \
      stream_exp;                              \
      (logger).log(LOG.str(), Logger::FATAL_); \
    }                                          \
    exit(1);                                   \
  }

#endif  // DSA_SDK_MODULE_LOGGER_H
