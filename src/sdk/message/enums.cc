#include "dsa_common.h"

#include "enums.h"

namespace dsa {

std::ostream &operator<<(std::ostream &os, MessageType type) {
  switch (type) {
    case MessageType::CLOSE:
      return os << std::string("CLOSE");
    case MessageType::SUBSCRIBE_REQUEST:
      return os << std::string("SUBSCRIBE_REQUEST");
    case MessageType::LIST_REQUEST:
      return os << std::string("LIST_REQUEST");
    case MessageType::INVOKE_REQUEST:
      return os << std::string("INVOKE_REQUEST");
    case MessageType::SET_REQUEST:
      return os << std::string("SET_REQUEST");
    case MessageType::SUBSCRIBE_RESPONSE:
      return os << std::string("SUBSCRIBE_RESPONSE");
    case MessageType::LIST_RESPONSE:
      return os << std::string("LIST_RESPONSE");
    case MessageType::INVOKE_RESPONSE:
      return os << std::string("INVOKE_RESPONSE");
    case MessageType::SET_RESPONSE:
      return os << std::string("SET_RESPONSE");
    case MessageType::HANDSHAKE0:
      return os << std::string("HANDSHAKE0");
    case MessageType::HANDSHAKE1:
      return os << std::string("HANDSHAKE1");
    case MessageType::HANDSHAKE2:
      return os << std::string("HANDSHAKE2");
    case MessageType::HANDSHAKE3:
      return os << std::string("HANDSHAKE3");
    case MessageType::ACK:
      return os << std::string("ACK");
    default:
      return os << std::string("INVALID");
  }
}

}
