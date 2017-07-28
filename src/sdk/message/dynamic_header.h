#ifndef DSA_SDK_DYNAMIC_HEADER_H
#define DSA_SDK_DYNAMIC_HEADER_H

#include <string>

#include "../util/exception.h"

namespace dsa {

class DynamicHeader {
 public:
  enum DynamicKey : uint8_t {  // https://github.com/iot-dsa-v2/docs/blob/master/protocol/Header-Structure.md
    Status = 0x00,
    SequenceId = 0x01,
    PageId = 0x02,
    AliasCount = 0x08,
    Priority = 0x10,
    NoStream = 0x11,
    Qos = 0x12,
    QueueSize = 0x14,
    QueueTime = 0x15,
    BasePath = 0x21,
    Skippable = 0x30,
    MaxPermission = 0x32,
    PermissionToken = 0x60,
    TargetPath = 0x80,
    SourcePath = 0x81
  };

  enum Offset {
    Key = 0,
    ByteValue = Key + sizeof(uint8_t),
    StringLength = Key + sizeof(uint8_t),
    StringValue = StringLength + sizeof(uint16_t)
  };

  static DynamicHeader *parse(const uint8_t *data,
                              size_t size) throw(const MessageParsingError &);

  uint8_t key() const { return _key; }

  // total size in bytes of this header
  uint16_t size() const { return _size; }

  virtual void write(uint8_t *data) const = 0;

 protected:
  uint8_t _key;
  uint16_t _size;

  DynamicHeader(uint8_t key, size_t size);
};

class DynamicStringHeader : public DynamicHeader {
 private:
  std::string _value;
  static const std::string BLANK_STRING;

 public:
  DynamicStringHeader(const uint8_t *data, uint16_t size, std::string str);
  DynamicStringHeader(uint8_t key, const std::string &str);
  const std::string &value() const;
  void write(uint8_t *data) const override;

  static const std::string &read_value(
      const std::unique_ptr<DynamicStringHeader> &header) throw() {
    if (header == nullptr) return BLANK_STRING;
    return header->value();
  }
  // return true when the length of content is changed
  static bool write_value(std::unique_ptr<DynamicStringHeader> &header,
                          DynamicKey key, const std::string &value) {
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

class DynamicByteHeader : public DynamicHeader {
 private:
  uint8_t _value;

 public:
  const uint8_t value() const { return _value; };
  explicit DynamicByteHeader(const uint8_t *data);
  DynamicByteHeader(uint8_t key, uint8_t value);
  void write(uint8_t *data) const override;

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

class DynamicIntHeader : public DynamicHeader {
 private:
  int32_t _value;

 public:
  const int32_t value() const { return _value; };
  explicit DynamicIntHeader(const uint8_t *data);
  DynamicIntHeader(uint8_t key, int32_t value);
  void write(uint8_t *data) const override;

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
      if (header != nullptr) {
        header.reset(new DynamicIntHeader(key, value));
        return true;
      }
      header.reset(new DynamicIntHeader(key, value));
    }
    return false;
  }
};

class DynamicBoolHeader : public DynamicHeader {
 protected:
 public:
  explicit DynamicBoolHeader(const uint8_t *data);
  explicit DynamicBoolHeader(uint8_t key);
  void write(uint8_t *data) const override;

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
