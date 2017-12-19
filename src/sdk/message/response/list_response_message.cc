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

ListResponseMessage::~ListResponseMessage() = default;

const string_& ListResponseMessage::get_pub_path() const {
  return DynamicStringHeader::read_value(pub_path);
}
void ListResponseMessage::set_pub_path(const string_& value) {
  if (DynamicStringHeader::write_value(pub_path, DynamicHeader::PUB_PATH,
                                       value)) {
    static_headers.message_size = 0;
  }
}

const bool ListResponseMessage::get_refreshed() const {
  return DynamicBoolHeader::read_value(refreshed);
}
void ListResponseMessage::set_refreshed(bool value) {
  if (DynamicBoolHeader::write_value(refreshed, DynamicHeader::REFRESHED,
                                     value)) {
    static_headers.message_size = 0;
  }
}

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
      string_ key(reinterpret_cast<const char*>(data), key_size);
      data += key_size;
      size -= key_size;
      // read value
      uint16_t value_size = read_16_t(data);
      data += sizeof(uint16_t);
      size -= sizeof(uint16_t);
      if (size < value_size) return;
      _raw_map[key] =
          make_ref_<VarBytes>(new RefCountBytes(data, data + value_size));
      data += value_size;
      size -= value_size;
    }
  }
}

ref_<VarMap> ListResponseMessage::get_parsed_map() const {
  VarMap* map = new VarMap();

  for (auto& it : _raw_map) {
    (*map)[it.first] = it.second->get_value();
  }

  return map->get_ref();
}

void ListResponseMessage::print_body(std::ostream& os) const {
  for (auto& it : _raw_map) {
    os << " " << it.first << ": ";
    if (it.second == nullptr) {
      os << "nullptr";
      continue;
    }
    if (it.second->size() > 0 && it.second->size() < 128) {
      Var& v = it.second->get_value();
      if (!v.is_null()) {
        os << v.to_json();
        continue;
      }
    }
    os << "SIZE:" << body->size();
  }
}

}  // namespace dsa
