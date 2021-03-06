#ifndef DSA_SDK_DYNAMIC_HEADER_H
#define DSA_SDK_DYNAMIC_HEADER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <memory>

#include "../util/exception.h"

namespace dsa {

class DynamicHeader {
 public:
  enum DynamicKey : uint8_t {  // https://github.com/iot-dsa-v2/docs/blob/master/protocol/Header-Structure.md
    STATUS = 0x00,
    SEQUENCE_ID = 0x01,
    PAGE_ID = 0x02,
    AUDIT_LOG = 0x04,
    ERROR_DETAIL = 0x05,
    ALIAS_COUNT = 0x08,
    PRIORITY = 0x10,
    NO_STREAM = 0x11,
    QOS = 0x12,
    QUEUE_SIZE = 0x14,
    QUEUE_TIME = 0x15,
    REFRESHED = 0x20,
    PUB_PATH = 0x21,
    SKIPPABLE = 0x30,
    MAX_PERMISSION = 0x32,
    ATTRIBUTE_FIELD = 0x41,
    PERMISSION_TOKEN = 0x60,
    TARGET_PATH = 0x80,
    SOURCE_PATH = 0x81
  };

  static std::unique_ptr<DynamicHeader> parse(
          const uint8_t *data, size_t size) throw(const MessageParsingError &);

  DynamicKey key() const { return _key; }

  // total size in bytes of this header
  uint16_t size() const { return _size; }

  virtual void write(uint8_t *data) const = 0;

  virtual ~DynamicHeader() = default;

 protected:
  DynamicKey _key;
  uint16_t _size;

  DynamicHeader(DynamicKey key, size_t size);
};

class DynamicStringHeader final : public DynamicHeader {
 private:
  string_ _value;
  static const string_ BLANK_STRING;

 public:
  DynamicStringHeader(const uint8_t *data, uint16_t size, string_ &&str);
  DynamicStringHeader(DynamicKey key, const string_ &str);
  const string_ &value() const;
  void write(uint8_t *data) const final;

  static const string_ &read_value(
      const std::unique_ptr<DynamicStringHeader> &header) throw() {
    if (header == nullptr) return BLANK_STRING;
    return header->value();
  }
  // return true when the length of content is changed
  static bool write_value(std::unique_ptr<DynamicStringHeader> &header,
                          DynamicKey key, const string_ &value) {
    if (value.empty()) {
      if (header != nullptr) {
        header.reset();
        return true;
      }
    } else {
      header.reset(new DynamicStringHeader(key, value));
      return true;
    }
    return false;
  }
};

class DynamicByteHeader final : public DynamicHeader {
 private:
  uint8_t _value;

 public:
  const uint8_t value() const { return _value; };
  explicit DynamicByteHeader(const uint8_t *data);
  DynamicByteHeader(DynamicKey key, uint8_t value);
  void write(uint8_t *data) const final;

  static uint8_t read_value(const std::unique_ptr<DynamicByteHeader> &header) {
    if (header == nullptr) return 0;
    return header->value();
  }
  // return true when the length of content is changed
  static bool write_value(std::unique_ptr<DynamicByteHeader> &header,
                          DynamicKey key, uint8_t value) {
    if (value == 0) {
      if (header != nullptr) {
        header.reset();
        return true;
      }
    } else {
      if (header != nullptr) {
        header.reset(new DynamicByteHeader(key, value));
        return true;
      }
      header.reset(new DynamicByteHeader(key, value));
    }
    return false;
  }
};

class DynamicIntHeader final : public DynamicHeader {
 private:
  int32_t _value;

 public:
  const int32_t value() const { return _value; };
  explicit DynamicIntHeader(const uint8_t *data);
  DynamicIntHeader(DynamicKey key, int32_t value);
  void write(uint8_t *data) const final;

  static int32_t read_value(const std::unique_ptr<DynamicIntHeader> &header) {
    if (header == nullptr) return 0;
    return header->value();
  }
  // return true when the length of content is changed
  static bool write_value(std::unique_ptr<DynamicIntHeader> &header,
                          DynamicKey key, int32_t value) {
    if (value == 0) {
      if (header != nullptr) {
        header.reset();
        return true;
      }
    } else {
      if (header == nullptr) {
        header.reset(new DynamicIntHeader(key, value));
        return true;
      }
      header.reset(new DynamicIntHeader(key, value));
    }
    return false;
  }
};

class DynamicBoolHeader final : public DynamicHeader {
 protected:
 public:
  explicit DynamicBoolHeader(const uint8_t *data);
  explicit DynamicBoolHeader(DynamicKey key);
  void write(uint8_t *data) const final;

  static bool read_value(const std::unique_ptr<DynamicBoolHeader> &header) {
    return (header != nullptr);
  }
  // return true when the length of content is changed
  static bool write_value(std::unique_ptr<DynamicBoolHeader> &header,
                          DynamicKey key, bool value) {
    if (value) {
      if (header == nullptr) {
        header.reset(new DynamicBoolHeader(key));
        return true;
      }
    } else {
      if (header != nullptr) {
        header.reset();
        return true;
      }
    }
    return false;
  }
};

}  // namespace dsa

#endif  // DSA_SDK_DYNAMIC_HEADER_H
