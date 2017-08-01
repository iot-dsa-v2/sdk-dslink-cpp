#include "dsa_common.h"

#include "dynamic_header.h"

#include <limits>

namespace dsa {

DynamicHeader::DynamicHeader(DynamicKey key, size_t size) : _key(key) {
  if (size > std::numeric_limits<uint16_t>::max())
    throw MessageParsingError("Header size too large");
  _size = uint16_t(size);
}

DynamicHeader *DynamicHeader::parse(const uint8_t *data, size_t size) throw(
    const MessageParsingError &) {
  switch (*data) {
    case Status:
    case SequenceId:
    case PageId:
    case AliasCount:
    case Priority:
    case Qos:
    case QueueSize:
    case QueueTime:
    case MaxPermission: {
      if (size >= 2) {
        return new DynamicByteHeader(data);
      }
      throw MessageParsingError("invalid size for DynamicByteHeader");
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
      throw MessageParsingError("invalid size for DynamicStringHeader");
    }
    case NoStream:
    case Skippable: {
      if (size >= 1) {
        return new DynamicBoolHeader(data);
      }
      throw MessageParsingError("invalid size for DynamicBoolHeader");
    }
    default:
      throw MessageParsingError("invalid dynamic header key");
  }
}

const std::string DynamicStringHeader::BLANK_STRING = "";

DynamicStringHeader::DynamicStringHeader(const uint8_t *data, uint16_t size,
                                         std::string str)
    : DynamicHeader(static_cast<DynamicKey>(*data), size), _value(std::move(str)) {}

DynamicStringHeader::DynamicStringHeader(DynamicKey key,
                                         const std::string &str)
    : DynamicHeader(key, str.length() + 3), _value(std::move(str)) {}

const std::string &DynamicStringHeader::value() const { return _value; }

void DynamicStringHeader::write(uint8_t *data) const {
  data[Key] = _key;
  uint16_t str_size = static_cast<uint16_t>(_value.length());
  memcpy(data + StringLength, &str_size, sizeof(str_size));
  memcpy(data + StringValue, _value.c_str(), str_size);
}

DynamicByteHeader::DynamicByteHeader(const uint8_t *data)
    : DynamicHeader(static_cast<DynamicKey>(*data), 2), _value(data[1]) {}

DynamicByteHeader::DynamicByteHeader(DynamicKey key, uint8_t value)
    : DynamicHeader(key, 2), _value(value) {}

void DynamicByteHeader::write(uint8_t *data) const {
  data[0] = key();
  data[1] = _value;
}

DynamicIntHeader::DynamicIntHeader(const uint8_t *data)
    : DynamicHeader(static_cast<DynamicKey>(*data), 5) {
  memcpy(&_value, data + 1, sizeof(_value));
}

DynamicIntHeader::DynamicIntHeader(DynamicKey key, int32_t value)
    : DynamicHeader(key, 5), _value(value) {}

void DynamicIntHeader::write(uint8_t *data) const {
  data[0] = key();
  memcpy(data + 1, &_value, sizeof(_value));
}

DynamicBoolHeader::DynamicBoolHeader(const uint8_t *data)
    : DynamicHeader(static_cast<DynamicKey>(*data), 1) {}

DynamicBoolHeader::DynamicBoolHeader(DynamicKey key)
    : DynamicHeader(key, 1) {}

void DynamicBoolHeader::write(uint8_t *data) const { data[0] = key(); }

}  // namespace dsa
