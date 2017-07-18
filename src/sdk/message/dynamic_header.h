#ifndef DSA_SDK_DYNAMIC_HEADER_H
#define DSA_SDK_DYNAMIC_HEADER_H

#include <cstdint>
#include <string>
namespace dsa {

class DynamicHeader {
 public:
  enum {  // https://github.com/iot-dsa-v2/docs/blob/master/protocol/Header-Structure.md
    STATUS = 0x00,
    SEQUENCE_ID = 0x01,
    PAGE_ID = 0x02,
    ALIAS_COUNT = 0x08,
    PRIORITY = 0x10,
    NO_STREAM = 0x11,
    QOS = 0x12,
    UPDATE_FREQUENCY = 0x13,
    QUQUE_SIZE = 0x14,
    QUEUE_TIME = 0x15,
    BASE_PATH = 0x21,
    SKIPPABLE = 0x30,
    MAX_PERMISSION = 0x32,
    PERMISSION_TOKEN = 0x60,
    TARGET_PATH = 0x80,
    SOURCE_PATH = 0x81
  };

  static DynamicHeader* parse(const uint8_t* data, size_t size);

  const uint8_t& name() const;
  // total size in bytes of this header
  const uint16_t& size() const;
  virtual void write(uint8_t * data) = 0;

 private:
  uint8_t _name;
  uint16_t _size;

 protected:
  DynamicHeader(const uint8_t key, const uint16_t size);
};

class DynamicStringHeader : public DynamicHeader {
 private:
  std::string _value;

 public:
  DynamicStringHeader(const uint8_t* data, const uint16_t size,
                      const std::string& str);
  DynamicStringHeader(const uint8_t name, const std::string& str);
  const std::string& value() const;
  void write(uint8_t * data);
};

class DynamicByteHeader : public DynamicHeader {
 private:
   uint16_t _value;

 public:
  const uint8_t& value() const;
  DynamicByteHeader(const uint8_t* data);
  DynamicByteHeader(const uint8_t name, const uint8_t value);
  void write(uint8_t * data);
};

class DynamicBoolHeader : public DynamicHeader {
 protected:
 public:
  DynamicBoolHeader(const uint8_t* data);
  DynamicBoolHeader(const uint8_t name);
  void write(uint8_t * data);
};

}  // namespace dsa

#endif  // DSA_SDK_DYNAMIC_HEADER_H
