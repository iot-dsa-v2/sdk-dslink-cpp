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

Message* Message::parse_message(const uint8_t* begin, const uint8_t* end) throw(
    const MessageParsingError&) {
  if (end - begin < StaticHeaders::TotalSize) {
    return nullptr;
  }

  auto type = MessageType(begin[StaticHeaders::TypeOffset]);

  switch (type) {
    case MessageType::SubscribeRequest:
      return new SubscribeRequestMessage(begin, end);
    case MessageType::ListRequest:
      return new ListRequestMessage(begin, end);
    case MessageType::InvokeRequest:
      return new InvokeRequestMessage(begin, end);
    case MessageType::SetRequest:
      return new SetRequestMessage(begin, end);
    case MessageType::SubscribeResponse:
      return new SubscribeResponseMessage(begin, end);
    case MessageType::ListResponse:
      return new ListResponseMessage(begin, end);
    case MessageType::InvokeResponse:
      return new InvokeResponseMessage(begin, end);
    case MessageType::SetResponse:
      return new SetResponseMessage(begin, end);
    default:
      throw MessageParsingError("invalid message type");
  }
}

}  // namespace dsa
