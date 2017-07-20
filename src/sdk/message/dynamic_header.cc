#include "dynamic_header.h"

namespace dsa {

DynamicHeader::DynamicHeader(const uint8_t key, const uint16_t size)
    : _key(key), _size(size) {}

DynamicHeader *DynamicHeader::parse(const uint8_t *data, size_t size) throw(
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
      throw new std::runtime_error("invalid size for DynamicByteHeader");
    }
    case BasePath:
    case PermissionToken:
    case TargetPath:
    case SourcePath: {
      if (size > 0) {
        uint8_t size1 = data[1];
        uint16_t total_size;
        if (size1 < 128) {
          total_size = size1 + 2;
          if (total_size <= size) {
            return new DynamicStringHeader(
                data, total_size, std::string((char *)data + 2, (size_t)size1));
          }

        } else {
          uint16_t size2 = ((size1 << 8) | data[2]) & 0x7fff;
          total_size = size1 + 3;
          if (total_size <= size) {
            return new DynamicStringHeader(
                data, total_size, std::string((char *)data + 3, (size_t)size2));
          }
        }
      }
      throw new std::runtime_error("invalid size for DynamicStringHeader");
    }
    case NoStream:
    case Skippable: {
      if (size >= 1) {
        return new DynamicBoolHeader(data);
      }
      throw new std::runtime_error("invalid size for DynamicBoolHeader");
    }
    default:
      throw new std::runtime_error("invalid dynamic header key");
  }
}

DynamicStringHeader::DynamicStringHeader(const uint8_t *data,
                                         const uint16_t size,
                                         const std::string &str)
    : DynamicHeader(*data, size), _value(str) {}
DynamicStringHeader::DynamicStringHeader(const uint8_t key,
                                         const std::string &str)
    : DynamicHeader(key,
                    str.length() > 127 ? str.length() + 3 : str.length() + 2),
      _value(str) {}
const std::string &DynamicStringHeader::value() const { return _value; }
void DynamicStringHeader::write(uint8_t *data) {
  data[0] = key();
  if (_value.length() > 127) {
    data[1] = _value.length();
    memcpy(data + 2, _value.c_str(), _value.length());
  } else {
    uint16_t size2 = _value.length() | 0x8000;
    data[1] = size2 >> 8;
    data[2] = size2 & 0xff;
    memcpy(data + 3, _value.c_str(), _value.length());
  }
}

DynamicByteHeader::DynamicByteHeader(const uint8_t *data)
    : DynamicHeader(*data, 2), _value(data[1]) {}

DynamicByteHeader::DynamicByteHeader(const uint8_t key, const uint8_t value)
    : DynamicHeader(key, 2), _value(value) {}

const uint8_t &DynamicByteHeader::value() const { return _value; }
void DynamicByteHeader::write(uint8_t *data) {
  data[0] = key();
  data[1] = _value;
}

DynamicBoolHeader::DynamicBoolHeader(const uint8_t *data)
    : DynamicHeader(*data, 1) {}

DynamicBoolHeader::DynamicBoolHeader(const uint8_t key)
    : DynamicHeader(key, 1) {}

void DynamicBoolHeader::write(uint8_t *data) { data[0] = key(); }

}  // namespace dsa
