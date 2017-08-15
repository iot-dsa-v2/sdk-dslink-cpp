#include "dsa_common.h"

#include "request/invoke_request_message.h"
#include "request/list_request_message.h"
#include "request/set_request_message.h"
#include "request/subscribe_request_message.h"

#include "response/invoke_response_message.h"
#include "response/list_response_message.h"
#include "response/set_response_message.h"
#include "response/subscribe_response_message.h"

namespace dsa {

Message* Message::parse_message(const uint8_t* data, size_t size) throw(
    const MessageParsingError&) {
  if (size < StaticHeaders::TotalSize) {
    return nullptr;
  }

  auto type = MessageType(data[StaticHeaders::TypeOffset]);

  switch (type) {
    case MessageType::SubscribeRequest:
      return new SubscribeRequestMessage(data, size);
    case MessageType::ListRequest:
      return new ListRequestMessage(data, size);
    case MessageType::InvokeRequest:
      return new InvokeRequestMessage(data, size);
    case MessageType::SetRequest:
      return new SetRequestMessage(data, size);
    case MessageType::SubscribeResponse:
      return new SubscribeResponseMessage(data, size);
    case MessageType::ListResponse:
      return new ListResponseMessage(data, size);
    case MessageType::InvokeResponse:
      return new InvokeResponseMessage(data, size);
    case MessageType::SetResponse:
      return new SetResponseMessage(data, size);
    default:
      throw MessageParsingError("invalid message type");
  }
}

}  // namespace dsa
