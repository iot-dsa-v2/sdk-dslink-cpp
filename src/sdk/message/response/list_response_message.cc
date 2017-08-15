#include "dsa_common.h"

#include "list_response_message.h"

namespace dsa {
ListResponseMessage::ListResponseMessage(const uint8_t* data, size_t size)
    : ResponseMessage(data, size) {
  parse_dynamic_headers(data + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}

ListResponseMessage::ListResponseMessage()
    : ResponseMessage(MessageType::ListResponse) {}

const std::string& ListResponseMessage::get_base_path() const {
  return DynamicStringHeader::read_value(base_path);
}
void ListResponseMessage::set_base_path(const std::string& value) {
  if (DynamicStringHeader::write_value(base_path, DynamicHeader::BasePath,
                                       value)) {
    static_headers.message_size = 0;
  }
}

const VariantMap& ListResponseMessage::get_map() {
  if (_parsed_map == nullptr) {
    VariantMap* map = new VariantMap();
    _parsed_map.reset(map);
    if (body != nullptr) {
      const uint8_t* data = body->data();
      size_t size = body->size();
      while (size > 4) {
        uint16_t key_size;
        uint16_t value_size;
        memcpy(&key_size, data, sizeof(uint16_t));
        data += sizeof(uint16_t);
        size -= sizeof(uint16_t);
        if (size < key_size) return *map;
        std::string key(reinterpret_cast<const char*>(data), key_size);
        data += key_size;
        size -= key_size;
        memcpy(&value_size, data, sizeof(uint16_t));
        data += sizeof(uint16_t);
        size -= sizeof(uint16_t);
        if (size < value_size) return *map;
        (*map)[key] = std::move(Variant::from_msgpack(data, size));
        data += value_size;
        size -= value_size;
      }
    }
    return *map;
  }
  return *_parsed_map;
}

}  // namespace dsa
