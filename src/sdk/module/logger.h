#ifndef DSA_SDK_MODULE_LOGGER_H_
#define DSA_SDK_MODULE_LOGGER_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <sstream>
#include "util/enable_ref.h"

namespace dsa {

class Logger: public DestroyableRef<Logger>{
 public:
  enum : uint8_t {
    ALL___ = 0x00,
    TRACE_ = 0x01,
    DEBUG_ = 0x02,
    INFO__ = 0x04,
    WARN__ = 0x08,
    ERROR_ = 0x10,
    SYS___ = 0x20,
    FATAL_ = 0x40,
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
  virtual void log(const string_& str, uint8_t level) = 0;
  virtual ~Logger() = default;

protected:
 void destroy_impl() override;
};
}

#define DSA_LOG(LEVEL, logger, stream_exp)  \
  if ((logger).level <= Logger::LEVEL) {    \
    std::stringstream LOG;                  \
    LOG << std::endl;                       \
    (logger).write_meta(LOG, #LEVEL);       \
    stream_exp;                             \
    (logger).log(LOG.str(), Logger::LEVEL); \
  }

#define LOG_ERROR(logger, stream_exp) DSA_LOG(ERROR_, logger, stream_exp)

#define LOG_WARN(logger, stream_exp) DSA_LOG(WARN__, logger, stream_exp)

#define LOG_INFO(logger, stream_exp) DSA_LOG(INFO__, logger, stream_exp)

#define LOG_SYSTEM(logger, stream_exp) DSA_LOG(SYS___, logger, stream_exp)

#define LOG_DEBUG(logger, stream_exp) DSA_LOG(DEBUG_, logger, stream_exp)

#define LOG_TRACE(logger, stream_exp) DSA_LOG(TRACE_, logger, stream_exp)

#define LOG_FATAL(stream_exp)                  \
  {                                            \
    std::stringstream LOG;                     \
    LOG << std::endl;                          \
    dsa::Logger& logger = Logger::_();         \
    if ((logger).level <= Logger::FATAL_) {    \
      (logger).write_meta(LOG, "FATAL_");      \
      stream_exp;                              \
      (logger).log(LOG.str(), Logger::FATAL_); \
    }                                          \
    exit(1);                                   \
  }

#endif  // DSA_SDK_MODULE_LOGGER_H_
