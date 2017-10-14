#include "dsa_common.h"

#include "list_response_message.h"
#include "util/little_endian.h"
#include "variant/variant.h"

namespace dsa {
ListResponseMessage::ListResponseMessage(const uint8_t* data, size_t size)
    : ResponseMessage(data, size) {
  parse_dynamic_data(data + StaticHeaders::TOTAL_SIZE,
                     static_headers.header_size - StaticHeaders::TOTAL_SIZE,
                     size - static_headers.header_size);
}

ListResponseMessage::ListResponseMessage()
    : ResponseMessage(MessageType::LIST_RESPONSE) {}

const std::string& ListResponseMessage::get_base_path() const {
  return DynamicStringHeader::read_value(base_path);
}
void ListResponseMessage::set_base_path(const std::string& value) {
  if (DynamicStringHeader::write_value(base_path, DynamicHeader::BASE_PATH,
                                       value)) {
    static_headers.message_size = 0;
  }
}

ListResponseMessage::~ListResponseMessage() = default;

void ListResponseMessage::parse() {
  if (body != nullptr) {
    const uint8_t* data = body->data();
    size_t size = body->size();
    while (size > 4) {
      // read key
      uint16_t key_size = read_16_t(data);
      data += sizeof(uint16_t);
      size -= sizeof(uint16_t);
      if (size < key_size) return;
      std::string key(reinterpret_cast<const char*>(data), key_size);
      data += key_size;
      size -= key_size;
      // read value
      uint16_t value_size = read_16_t(data);
      data += sizeof(uint16_t);
      size -= sizeof(uint16_t);
      if (size < value_size) return;
      _raw_map[key] = make_ref_<const IntrusiveBytes>(data, data + value_size);
      data += value_size;
      size -= value_size;
    }
  }
}

ref_<VariantMap> ListResponseMessage::get_map() const {
  VariantMap* map = new VariantMap();

  for (auto& it : _raw_map) {
    (*map)[it.first] =
        Variant::from_msgpack(it.second->data(), it.second->size());
  }

  return map->get_ref();
}

}  // namespace dsa
