#include "dynamic_header.h"

#include <limits>

namespace dsa {

DynamicHeader::DynamicHeader(uint8_t key, size_t size) : _key(key) {
  if (size > std::numeric_limits<uint16_t>::max())
    throw std::runtime_error("Header size too large");
  _size = uint16_t(size);
}

DynamicHeader *DynamicHeader::parse(const uint8_t *data, uint16_t size) throw(
    const std::runtime_error &) {
  switch (*data) {
    case Status:
    case SequenceId:
    case PageId:
    case AliasCount:
    case Priority:
    case Qos:
    case UpdateFrequency:
    case QueueSize:
    case QueueTime:
    case MaxPermission: {
      if (size >= 2) {
        return new DynamicByteHeader(data);
      }
      throw std::runtime_error("invalid size for DynamicByteHeader");
    }
    case BasePath:
    case PermissionToken:
    case TargetPath:
    case SourcePath: {
      if (size > 0) {
        uint16_t str_size;
        memcpy(&str_size, data + 1, sizeof(str_size));
        if (str_size + 3 <= size) {
          return new DynamicStringHeader(
              data, str_size + 3,
              std::string((char *)data + 3, (size_t)str_size));
        }
      }
      throw std::runtime_error("invalid size for DynamicStringHeader");
    }
    case NoStream:
    case Skippable: {
      if (size >= 1) {
        return new DynamicBoolHeader(data);
      }
      throw std::runtime_error("invalid size for DynamicBoolHeader");
    }
    default:
      throw std::runtime_error("invalid dynamic header key");
  }
}

DynamicStringHeader::DynamicStringHeader(const uint8_t *data, uint16_t size,
                                         std::string str)
    : DynamicHeader(*data, size), _value(std::move(str)) {}

DynamicStringHeader::DynamicStringHeader(const uint8_t key, std::string str)
    : DynamicHeader(key, str.length() + 3), _value(std::move(str)) {}

const std::string &DynamicStringHeader::value() const { return _value; }

void DynamicStringHeader::write(uint8_t *data) const {
  data[Key] = _key;
  uint16_t str_size = _value.length();
  memcpy(data + StringLength, &str_size, sizeof(str_size));
  memcpy(data + StringValue, _value.c_str(), str_size);
}

DynamicByteHeader::DynamicByteHeader(const uint8_t *data)
    : DynamicHeader(*data, 2), _value(data[1]) {}

DynamicByteHeader::DynamicByteHeader(const uint8_t key, const uint8_t value)
    : DynamicHeader(key, 2), _value(value) {}

void DynamicByteHeader::write(uint8_t *data) const {
  data[0] = key();
  data[1] = _value;
}

DynamicIntHeader::DynamicIntHeader(const uint8_t *data)
    : DynamicHeader(*data, 5) {
  memcpy(&_value, data + 1, sizeof(_value));
}

DynamicIntHeader::DynamicIntHeader(const uint8_t key, const int32_t value)
    : DynamicHeader(key, 5), _value(value) {}

void DynamicIntHeader::write(uint8_t *data) {
  data[0] = key();
  memcpy(data + 1, &_value, sizeof(_value));
}

DynamicBoolHeader::DynamicBoolHeader(const uint8_t *data)
    : DynamicHeader(*data, 1) {}

DynamicBoolHeader::DynamicBoolHeader(const uint8_t key)
    : DynamicHeader(key, 1) {}

void DynamicBoolHeader::write(uint8_t *data) const { data[0] = key(); }

}  // namespace dsa
