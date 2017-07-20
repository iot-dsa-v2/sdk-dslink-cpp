#ifndef DSA_SDK_DYNAMIC_HEADER_H
#define DSA_SDK_DYNAMIC_HEADER_H

#include <cstdint>
#include <string>
namespace dsa {

class DynamicHeader {
 public:
  enum {  // https://github.com/iot-dsa-v2/docs/blob/master/protocol/Header-Structure.md
    Status = 0x00,
    SequenceId = 0x01,
    PageId = 0x02,
    AliasCount = 0x08,
    Priority = 0x10,
    NoStream = 0x11,
    Qos = 0x12,
    UpdateFrequency = 0x13,
    QueueSize = 0x14,
    QueueTime = 0x15,
    BasePath = 0x21,
    Skippable = 0x30,
    MaxPermission = 0x32,
    PermissionToken = 0x60,
    TargetPath = 0x80,
    SourcePath = 0x81
  };

  static DynamicHeader* parse(const uint8_t* data,
                              size_t size) throw(const std::runtime_error&);

  const uint8_t& key() const { return _key; }
  // total size in bytes of this header
  const uint16_t& size() const { return _size; }
  virtual void write(uint8_t* data) = 0;

 private:
  uint8_t _key;
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
  DynamicStringHeader(const uint8_t key, const std::string& str);
  const std::string& value() const;
  void write(uint8_t* data);
};

class DynamicByteHeader : public DynamicHeader {
 private:
  uint16_t _value;

 public:
  const uint8_t& value() const;
  DynamicByteHeader(const uint8_t* data);
  DynamicByteHeader(const uint8_t key, const uint8_t value);
  void write(uint8_t* data);
};

class DynamicBoolHeader : public DynamicHeader {
 protected:
 public:
  DynamicBoolHeader(const uint8_t* data);
  DynamicBoolHeader(const uint8_t key);
  void write(uint8_t* data);
};

}  // namespace dsa

#endif  // DSA_SDK_DYNAMIC_HEADER_H
