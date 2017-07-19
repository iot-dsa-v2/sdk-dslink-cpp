#include "dynamic_header.h"

namespace dsa {

DynamicHeader::DynamicHeader(const uint8_t key, const uint16_t size)
    : _key(key), _size(size) {}

const uint8_t &DynamicHeader::key() const { return _key; }

// total size in bytes of this header
const uint16_t &DynamicHeader::size() const { return _size; }

DynamicHeader *DynamicHeader::parse(const uint8_t *data, size_t size) {
  switch (*data) {
    case STATUS:
    case SEQUENCE_ID:
    case PAGE_ID:
    case ALIAS_COUNT:
    case PRIORITY:
    case QOS:
    case UPDATE_FREQUENCY:
    case QUQUE_SIZE:
    case QUEUE_TIME:
    case MAX_PERMISSION: {
      if (size >= 2) {
        return new DynamicByteHeader(data);
      }
      break;
    }
    case BASE_PATH:
    case PERMISSION_TOKEN:
    case TARGET_PATH:
    case SOURCE_PATH: {
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
      break;
    }
    case NO_STREAM:
    case SKIPPABLE: {
      if (size >= 1) {
        return new DynamicBoolHeader(data);
      }
      break;
    }
  }
  return nullptr;
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
