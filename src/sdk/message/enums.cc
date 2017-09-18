#include "dsa_common.h"

#include "enums.h"

using namespace dsa;

std::ostream &operator<<(std::ostream &os, MessageType type) {
  switch (type) {
    case MessageType::CLOSE:
      return os << "CLOSE";
    case MessageType::SUBSCRIBE_REQUEST:
      return os << "SUBSCRIBE_REQUEST";
    case MessageType::LIST_REQUEST:
      return os << "LIST_REQUEST";
    case MessageType::INVOKE_REQUEST:
      return os << "INVOKE_REQUEST";
    case MessageType::SET_REQUEST:
      return os << "SET_REQUEST";
    case MessageType::SUBSCRIBE_RESPONSE:
      return os << "SUBSCRIBE_RESPONSE";
    case MessageType::LIST_RESPONSE:
      return os << "LIST_RESPONSE";
    case MessageType::INVOKE_RESPONSE:
      return os << "INVOKE_RESPONSE";
    case MessageType::SET_RESPONSE:
      return os << "SET_RESPONSE";
    case MessageType::HANDSHAKE0:
      return os << "HANDSHAKE0";
    case MessageType::HANDSHAKE1:
      return os << "HANDSHAKE1";
    case MessageType::HANDSHAKE2:
      return os << "HANDSHAKE2";
    case MessageType::HANDSHAKE3:
      return os << "HANDSHAKE3";
    case MessageType::ACK:
      return os << "ACK";
    default:
      return os << "INVALID";
  }
}