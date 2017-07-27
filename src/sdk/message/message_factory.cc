#include "dsa_common.h"

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

Message* parseMessage(const SharedBuffer& buffer) throw(
const MessageParsingError&) {
  if (buffer.size < StaticHeaders::TotalSize) {
    return nullptr;
  }
  uint8_t type = buffer.data[StaticHeaders::TypeOffset];

  switch (type) {
    case MessageType::SubscribeRequest:
      return new SubscribeRequestMessage(buffer);
    case MessageType::ListRequest:
      return new ListRequestMessage(buffer);
    case MessageType::InvokeRequest:
      return new InvokeRequestMessage(buffer);
    case MessageType::SetRequest:
      return new SetRequestMessage(buffer);
    case MessageType::SubscribeResponse:
      return new SubscribeResponseMessage(buffer);
    case MessageType::ListResponse:
      return new ListResponseMessage(buffer);
    case MessageType::InvokeResponse:
      return new InvokeResponseMessage(buffer);
    case MessageType::SetResponse:
      return new SetResponseMessage(buffer);
    default:
      throw MessageParsingError("invalid message type");
  }
}

}  // namespace dsa
