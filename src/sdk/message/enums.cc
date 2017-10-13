#include "dsa_common.h"

#include "enums.h"

namespace dsa {

const char* to_str(dsa::MessageType type) {
  switch (type) {
    case MessageType::CLOSE:
      return "CLOSE";
    case MessageType::SUBSCRIBE_REQUEST:
      return "SUBSCRIBE_REQUEST";
    case MessageType::LIST_REQUEST:
      return "LIST_REQUEST";
    case MessageType::INVOKE_REQUEST:
      return "INVOKE_REQUEST";
    case MessageType::SET_REQUEST:
      return "SET_REQUEST";
    case MessageType::SUBSCRIBE_RESPONSE:
      return "SUBSCRIBE_RESPONSE";
    case MessageType::LIST_RESPONSE:
      return "LIST_RESPONSE";
    case MessageType::INVOKE_RESPONSE:
      return "INVOKE_RESPONSE";
    case MessageType::SET_RESPONSE:
      return "SET_RESPONSE";
    case MessageType::HANDSHAKE0:
      return "HANDSHAKE0";
    case MessageType::HANDSHAKE1:
      return "HANDSHAKE1";
    case MessageType::HANDSHAKE2:
      return "HANDSHAKE2";
    case MessageType::HANDSHAKE3:
      return "HANDSHAKE3";
    case MessageType::ACK:
      return "ACK";
    default:
      return "INVALID";
  }
}

}
