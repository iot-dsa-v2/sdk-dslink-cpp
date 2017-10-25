#include "dsa_common.h"

#include "ack_message.h"

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
  if (size < StaticHeaders::TOTAL_SIZE) {
    return MessageRef();
  }

  auto type = MessageType(data[StaticHeaders::TYPE_OFFSET]);

  switch (type) {
    case MessageType::SUBSCRIBE_RESPONSE:
      return MessageRef(new SubscribeResponseMessage(data, size));
    case MessageType::LIST_RESPONSE:
      return MessageRef(new ListResponseMessage(data, size));
    case MessageType::INVOKE_RESPONSE:
      return MessageRef(new InvokeResponseMessage(data, size));
    case MessageType::SET_RESPONSE:
      return MessageRef(new SetResponseMessage(data, size));
    case MessageType::ACK:
      return MessageRef(new AckMessage(data, size));
    case MessageType::CLOSE_REQUEST:
      return MessageRef(new RequestMessage(data, size));
    case MessageType::PING:
      return MessageRef(new Message(MessageType::PING));
    case MessageType::SUBSCRIBE_REQUEST:
      return MessageRef(new SubscribeRequestMessage(data, size));
    case MessageType::LIST_REQUEST:
      return MessageRef(new ListRequestMessage(data, size));
    case MessageType::INVOKE_REQUEST:
      return MessageRef(new InvokeRequestMessage(data, size));
    case MessageType::SET_REQUEST:
      return MessageRef(new SetRequestMessage(data, size));
    case MessageType::HANDSHAKE0:
      return MessageRef(new HandshakeF0Message(data, size));
    case MessageType::HANDSHAKE1:
      return MessageRef(new HandshakeF1Message(data, size));
    case MessageType::HANDSHAKE2:
      return MessageRef(new HandshakeF2Message(data, size));
    case MessageType::HANDSHAKE3:
      return MessageRef(new HandshakeF3Message(data, size));
    case MessageType::INVALID:
      break;
  }
  throw MessageParsingError("invalid message type");
}

}  // namespace dsa
