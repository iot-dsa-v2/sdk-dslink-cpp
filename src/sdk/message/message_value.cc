#include "dsa_common.h"

#include "message_value.h"

#include "util/little_endian.h"

namespace dsa {

MessageValue::MessageValue(const Variant& value) : value(value) {}
MessageValue::MessageValue(Variant&& value) : value(std::move(value)) {}

MessageValue::MessageValue(const uint8_t* data, size_t size) {
  parse(data, size);
}

MessageValue::MessageValue(Variant&& value, const std::string& ts)
    : value(std::move(value)), meta({{"ts", Variant(ts)}}) {}

void MessageValue::parse(const uint8_t* data, size_t size) {
  if (size < 2) {
    return;
  }
  uint16_t meta_size;
  memcpy(&meta_size, data, sizeof(uint16_t));
  data += sizeof(meta_size);
  size -= sizeof(meta_size);

  if (meta_size > size) {
    return;
  }

  meta = std::move(Variant::from_msgpack(data, meta_size));
  data += meta_size;
  size -= meta_size;
  if (size > 0) {
    value = std::move(Variant::from_msgpack(data, size));
  }
}

BytesRef MessageValue::to_msgpack() const {
  std::vector<uint8_t> meta_bytes;
  std::vector<uint8_t> value_bytes;

  if (!value.is_null()) {
    value_bytes = value.to_msgpack();
  }

  if (meta.is_map()) {
    meta_bytes = meta.to_msgpack();
  }

  if (meta_bytes.size() > MAX_META_SIZE) {
    throw std::runtime_error("value meta data is too big");
  }

  auto merged = new IntrusiveBytes();
  merged->resize(2);
  merged->reserve(2 + meta_bytes.size() + value_bytes.size());

  write_16_t(merged->data(), meta_bytes.size());
  merged->insert(merged->end(), meta_bytes.begin(), meta_bytes.end());
  merged->insert(merged->end(), value_bytes.begin(), value_bytes.end());
  return BytesRef(merged);
}
}  // namespace dsa
