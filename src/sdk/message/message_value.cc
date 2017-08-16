#include "dsa_common.h"

#include "message_value.h"

namespace dsa {
MessageValue::MessageValue(const uint8_t* data, size_t size) {
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
  if (size) {
    value = std::move(Variant::from_msgpack(data, size));
  }
}

MessageValue::MessageValue(Variant &value) :value(value){

}
MessageValue::MessageValue(Variant &value, const std::string &ts)
    : value(value), meta(new VariantMap{{std::string("ts"), Variant(ts)}}) {
}

}  // namespace dsa
