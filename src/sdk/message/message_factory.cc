#include "dsa_common.h"

#include "request/invoke_request_message.h"
#include "request/list_request_message.h"
#include "request/set_request_message.h"
#include "request/subscribe_request_message.h"

#include "response/invoke_response_message.h"
#include "response/list_response_message.h"
#include "response/set_response_message.h"
#include "response/subscribe_response_message.h"

#include "handshake/f0_message.h"
#include "handshake/f1_message.h"
#include "handshake/f2_message.h"
#include "handshake/f3_message.h"

namespace dsa {

MessageRef Message::parse_message(const uint8_t* data, size_t size) throw(
    const MessageParsingError&) {
  if (size < StaticHeaders::TotalSize) {
    return nullptr;
  }

  auto type = MessageType(data[StaticHeaders::TypeOffset]);

  switch (type) {
    case MessageType::SUBSCRIBE_REQUEST:
      return new SubscribeRequestMessage(data, size);
    case MessageType::LIST_REQUEST:
      return new ListRequestMessage(data, size);
    case MessageType::INVOKE_REQUEST:
      return new InvokeRequestMessage(data, size);
    case MessageType::SET_REQUEST:
      return new SetRequestMessage(data, size);
    case MessageType::SUBSCRIBE_RESPONSE:
      return new SubscribeResponseMessage(data, size);
    case MessageType::LIST_RESPONSE:
      return new ListResponseMessage(data, size);
    case MessageType::INVOKE_RESPONSE:
      return new InvokeResponseMessage(data, size);
    case MessageType::SET_RESPONSE:
      return new SetResponseMessage(data, size);
    case MessageType::HANDSHAKE0:
      return new HandshakeF0Message(data, size);
    case MessageType::HANDSHAKE1:
      return new HandshakeF1Message(data, size);
    case MessageType::HANDSHAKE2:
      return new HandshakeF2Message(data, size);
    case MessageType::HANDSHAKE3:
      return new HandshakeF3Message(data, size);
    default:
      throw MessageParsingError("invalid message type");
  }
}

}  // namespace dsa
