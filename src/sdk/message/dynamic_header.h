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

  enum Offset {
    Key = 0,
    ByteValue = Key + sizeof(uint8_t),
    StringLength = Key + sizeof(uint8_t),
    StringValue = StringLength + sizeof(uint16_t)
  };

  static DynamicHeader* parse(const uint8_t* data,
                              uint16_t size) throw(const std::runtime_error&);

  uint8_t key() const { return _key; }

  // total size in bytes of this header
  uint16_t size() const { return _size; }

  virtual void write(uint8_t* data) const = 0;

 protected:
  uint8_t _key;
  uint16_t _size;

  DynamicHeader(uint8_t key, size_t size);
};

class DynamicStringHeader : public DynamicHeader {
 private:
  std::string _value;

 public:
  DynamicStringHeader(const uint8_t* data, uint16_t size, std::string str);
  DynamicStringHeader(uint8_t key, std::string str);
  const std::string& value() const;
  void write(uint8_t* data) const override;
};

class DynamicByteHeader : public DynamicHeader {
 private:
  uint8_t _value;

 public:
  const uint8_t value() const { return _value; };
  explicit DynamicByteHeader(const uint8_t* data);
  DynamicByteHeader(uint8_t key, uint8_t value);
  void write(uint8_t* data) const override;
};

class DynamicIntHeader : public DynamicHeader {
 private:
  int32_t _value;

 public:
  const int32_t value() const { return _value; };
  explicit DynamicIntHeader(const uint8_t* data);
  DynamicIntHeader(uint8_t key, int32_t value);
  void write(uint8_t* data) constoverride;
};

class DynamicBoolHeader : public DynamicHeader {
 protected:
 public:
  explicit DynamicBoolHeader(const uint8_t* data);
  explicit DynamicBoolHeader(uint8_t key);
  void write(uint8_t* data) const override;
};

}  // namespace dsa

#endif  // DSA_SDK_DYNAMIC_HEADER_H
