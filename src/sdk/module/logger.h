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
    FINE__ = 0x04,
    WARN__ = 0x08,
    INFO__ = 0x10,
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

  virtual void write_meta(std::ostream& stream, const char* level,
                          const string_& log_name) = 0;
  virtual void log(const string_& str, uint8_t level) = 0;
  virtual ~Logger() = default;

 protected:
  void destroy_impl() override;
};
}

#define DSA_LOG(LEVEL, LEVEL_NAME, stream_exp, log_name)      \
  if ((dsa::Logger::_()).level <= Logger::LEVEL) {            \
    std::stringstream LOG;                                    \
    LOG << std::endl;                                         \
    (dsa::Logger::_()).write_meta(LOG, LEVEL_NAME, log_name); \
    stream_exp;                                               \
    (dsa::Logger::_()).log(LOG.str(), Logger::LEVEL);         \
  }

#define LOG_ERROR(log_name, stream_exp) \
  DSA_LOG(ERROR_, "Error ", stream_exp, log_name)

#define LOG_WARN(log_name, stream_exp) \
  DSA_LOG(WARN__, "Warn  ", stream_exp, log_name)

#define LOG_FINE(log_name, stream_exp) \
  DSA_LOG(FINE__, "Info  ", stream_exp, log_name)

#define LOG_INFO(log_name, stream_exp) \
  DSA_LOG(INFO__, "Sys   ", stream_exp, log_name)

#define LOG_ADMIN(log_name, stream_exp) \
  DSA_LOG(ADMIN___, "Admin ", stream_exp, log_name)

#define LOG_DEBUG(log_name, stream_exp) \
  DSA_LOG(DEBUG_, "Debug ", stream_exp, log_name)

#define LOG_TRACE(log_name, stream_exp) \
  DSA_LOG(TRACE_, "Trace ", stream_exp, log_name)

#define LOG_FATAL(log_name, stream_exp)             \
  {                                                 \
    std::stringstream LOG;                          \
    LOG << std::endl;                               \
    dsa::Logger& logger = Logger::_();              \
    if ((logger).level <= Logger::FATAL_) {         \
      (logger).write_meta(LOG, "Fatal ", log_name); \
      stream_exp;                                   \
      (logger).log(LOG.str(), Logger::FATAL_);      \
    }                                               \
    exit(1);                                        \
  }

#endif  // DSA_SDK_MODULE_LOGGER_H
