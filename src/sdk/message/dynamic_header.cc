#include "dsa_common.h"

#include "dynamic_header.h"

#include <cstring>
#include <algorithm>

#include "util/little_endian.h"

namespace dsa {

DynamicHeader::DynamicHeader(DynamicKey key, size_t size) : _key(key) {
  if (size > std::numeric_limits<uint16_t>::max())
    throw MessageParsingError("Header size too large");
  _size = uint16_t(size);
}

std::unique_ptr<DynamicHeader> DynamicHeader::parse(const uint8_t *data, size_t size) throw(
    const MessageParsingError &) {
  switch (*data) {
    case STATUS:
    case ALIAS_COUNT:
    case QOS:
    case MAX_PERMISSION: {
      if (size >= 2) {
        return std::unique_ptr<DynamicByteHeader>(new DynamicByteHeader(data));
      }
      throw MessageParsingError("invalid size for DynamicByteHeader");
    }
    case SEQUENCE_ID:
    case PAGE_ID:
    case QUEUE_SIZE:
    case QUEUE_TIME: {
      if (size >= 5) {
        return std::unique_ptr<DynamicIntHeader>(new DynamicIntHeader(data));
      }
      throw MessageParsingError("invalid size for DynamicByteHeader");
    }

    case BASE_PATH:
    case ATTRIBUTE_FIELD:
    case PERMISSION_TOKEN:
    case TARGET_PATH:
    case SOURCE_PATH: {
      if (size > 0) {
        uint16_t str_size = read_16_t(data + 1);
        if (str_size + 3 <= size) {
          return std::unique_ptr<DynamicStringHeader> (new DynamicStringHeader(
              data, str_size + 3,
              string_((char *)data + 3, (size_t)str_size)));
        }
      }
      throw MessageParsingError("invalid size for DynamicStringHeader");
    }
    case PRIORITY:
    case NO_STREAM:
    case SKIPPABLE: {
      if (size >= 1) {
        return std::unique_ptr<DynamicBoolHeader>(new DynamicBoolHeader(data));
      }
      throw MessageParsingError("invalid size for DynamicBoolHeader");
    }
    default:
      throw MessageParsingError("invalid dynamic header key");
  }
}

const string_ DynamicStringHeader::BLANK_STRING = "";

DynamicStringHeader::DynamicStringHeader(const uint8_t *data, uint16_t size,
                                         string_ &&str)
    : DynamicHeader(static_cast<DynamicKey>(*data), size),
      _value(std::move(str)) {}

DynamicStringHeader::DynamicStringHeader(DynamicKey key, const string_ &str)
    : DynamicHeader(key, str.length() + 3), _value(str) {}

const string_ &DynamicStringHeader::value() const { return _value; }

void DynamicStringHeader::write(uint8_t *data) const {
  data[0] = _key;
  write_str_with_len(&data[1], _value);
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
  _value = read_32_t(data + 1);
}

DynamicIntHeader::DynamicIntHeader(DynamicKey key, int32_t value)
    : DynamicHeader(key, 5), _value(value) {}

void DynamicIntHeader::write(uint8_t *data) const {
  data[0] = key();
  write_32_t(data + 1, _value);
}

DynamicBoolHeader::DynamicBoolHeader(const uint8_t *data)
    : DynamicHeader(static_cast<DynamicKey>(*data), 1) {}

DynamicBoolHeader::DynamicBoolHeader(DynamicKey key) : DynamicHeader(key, 1) {}

void DynamicBoolHeader::write(uint8_t *data) const { data[0] = key(); }

}  // namespace dsa
