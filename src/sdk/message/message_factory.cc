#include "message_factory.h"

#include "request/invoke_request_message.h"
#include "request/list_request_message.h"
#include "request/set_request_message.h"
#include "request/subscribe_request_message.h"

#include "response/invoke_response_message.h"
#include "response/list_response_message.h"
#include "response/set_response_message.h"
#include "response/subscribe_response_message.h"

namespace dsa {

Message* parseMessage(const MessageBuffer& buffer) throw(
const std::exception&) {
  if (buffer.size < StaticHeaders::TotalSize) {
    return nullptr;
  }
  uint8_t type = buffer.data[StaticHeaders::TypeOffset];

  switch (type) {
    case Message::SUBSCRIBE_REQUEST_TYPE:
      return new SubscribeRequestMessage(buffer);
    case Message::LIST_REQUEST_TYPE:
      return new ListRequestMessage(buffer);
    case Message::INVOKE_REQUEST_TYPE:
      return new InvokeRequestMessage(buffer);
    case Message::SET_REQUEST_TYPE:
      return new SetRequestMessage(buffer);
    case Message::SUBSCRIBE_RESPONSE_TYPE:
      return new SubscribeResponseMessage(buffer);
    case Message::LIST_RESPONSE_TYPE:
      return new ListResponseMessage(buffer);
    case Message::INVOKE_RESPONSE_TYPE:
      return new InvokeResponseMessage(buffer);
    case Message::SET_RESPONSE_TYPE:
      return new SetResponseMessage(buffer);
    default:
      throw std::runtime_error("stuff");
  }
}

}  // namespace dsa
