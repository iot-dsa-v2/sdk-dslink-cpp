#include "dsa_common.h"

#include "enums.h"

namespace dsa {

std::ostream &operator<<(std::ostream &os, MessageType type) {
  switch (type) {
    case MessageType::CLOSE_REQUEST:
      return os << string_("CLOSE");
    case MessageType::SUBSCRIBE_REQUEST:
      return os << string_("SUBSCRIBE_REQUEST");
    case MessageType::LIST_REQUEST:
      return os << string_("LIST_REQUEST");
    case MessageType::INVOKE_REQUEST:
      return os << string_("INVOKE_REQUEST");
    case MessageType::SET_REQUEST:
      return os << string_("SET_REQUEST");
    case MessageType::SUBSCRIBE_RESPONSE:
      return os << string_("SUBSCRIBE_RESPONSE");
    case MessageType::LIST_RESPONSE:
      return os << string_("LIST_RESPONSE");
    case MessageType::INVOKE_RESPONSE:
      return os << string_("INVOKE_RESPONSE");
    case MessageType::SET_RESPONSE:
      return os << string_("SET_RESPONSE");
    case MessageType::HANDSHAKE0:
      return os << string_("HANDSHAKE0");
    case MessageType::HANDSHAKE1:
      return os << string_("HANDSHAKE1");
    case MessageType::HANDSHAKE2:
      return os << string_("HANDSHAKE2");
    case MessageType::HANDSHAKE3:
      return os << string_("HANDSHAKE3");
    case MessageType::ACK:
      return os << string_("ACK");
    default:
      return os << string_("INVALID");
  }
}

}
