#include "dsa_common.h"

#include "invoke_response_message.h"

#include "variant/variant.h"

namespace dsa {
InvokeResponseMessage::InvokeResponseMessage(const uint8_t* data, size_t size)
    : ResponseMessage(data, size) {
  parse_dynamic_data(data + StaticHeaders::TOTAL_SIZE,
                     static_headers.header_size - StaticHeaders::TOTAL_SIZE,
                     size - static_headers.header_size);
}

InvokeResponseMessage::InvokeResponseMessage()
    : ResponseMessage(MessageType::INVOKE_RESPONSE) {}

void InvokeResponseMessage::set_value(const Var& value) {
  set_body(new RefCountBytes(value.to_msgpack()));
}

}  // namespace dsa
