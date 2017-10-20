#include "dsa_common.h"

#include "invoke_request_message.h"

#include "variant/variant.h"

namespace dsa {

InvokeRequestMessage::InvokeRequestMessage(const uint8_t* data, size_t size)
    : RequestMessage(data, size) {
  parse_dynamic_data(data + StaticHeaders::TOTAL_SIZE,
                     static_headers.header_size - StaticHeaders::TOTAL_SIZE,
                     size - static_headers.header_size);
}

InvokeRequestMessage::InvokeRequestMessage()
    : RequestMessage(MessageType::INVOKE_REQUEST) {}

void InvokeRequestMessage::set_value(const Var& value) {
  set_body(new IntrusiveBytes(value.to_msgpack()));
}

}  // namespace dsa
