#include "dsa_common.h"

#include "subscribe_response_message.h"

namespace dsa {
SubscribeResponseMessage::SubscribeResponseMessage(const SharedBuffer& buffer)
    : ResponseMessage(buffer) {
  parse_dynamic_headers(buffer.data + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}

SubscribeResponseMessage::SubscribeResponseMessage()
    : ResponseMessage(MessageType::SubscribeResponse) {}

const ParsedMessageValue* SubscribeResponseMessage::get_value() {
  if (body != nullptr) {
    _parsed_value.reset(new ParsedMessageValue(*body));
  }
  return _parsed_value.get();
}

}  // namespace dsa
