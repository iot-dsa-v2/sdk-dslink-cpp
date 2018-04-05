#include "dsa_common.h"

#include "util/enums.h"

#include <iostream>

namespace dsa {

std::ostream &operator<<(std::ostream &os, MessageType type) {
  switch (type) {
    case MessageType::CLOSE_REQUEST:
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
    case MessageType::PING:
      return os << "PING";
    default:
      return os << "INVALID";
  }
}

const char *PermissionName::convert(PermissionLevel permission_level) {
  switch (permission_level) {
    case PermissionLevel::NONE:
      return PermissionName::NONE;
    case PermissionLevel::LIST:
      return PermissionName::LIST;
    case PermissionLevel::READ:
      return PermissionName::READ;
    case PermissionLevel::WRITE:
      return PermissionName::WRITE;
    case PermissionLevel::CONFIG:
      return PermissionName::CONFIG;
    case PermissionLevel::NEVER:
      return PermissionName::NEVER;
    default:
      return "Unknown Permission Level";
  }
}
PermissionLevel PermissionName::parse(string_ str) {
  if (str == PermissionName::CONFIG) {
    return PermissionLevel::CONFIG;
  } else if (str == PermissionName::WRITE) {
    return PermissionLevel::WRITE;
  } else if (str == PermissionName::READ) {
    return PermissionLevel::READ;
  } else if (str == PermissionName::LIST) {
    return PermissionLevel::LIST;
  } else if (str == PermissionName::NONE) {
    return PermissionLevel::NONE;
  }
  return PermissionLevel::NEVER;
}

const char *to_string(Status status) {
  switch (status) {
    case Status::OK:
      return "OK";
    case Status::INITIALIZING:
      return "INITIALIZING";
    case Status::REFRESHED:
      return "REFRESHED";
    case Status::NOT_AVAILABLE:
      return "NOT_AVAILABLE";
    case Status::DROPPED:
      return "DROPPED";
    case Status::DONE:
      return "DONE";
    case Status::DISCONNECTED:
      return "DISCONNECTED";
    case Status::PERMISSION_DENIED:
      return "PERMISSION_DENIED";
    case Status::NOT_SUPPORTED:
      return "NOT_SUPPORTED";
    case Status::INVALID_MESSAGE:
      return "INVALID_MESSAGE";
    case Status::INVALID_PARAMETER:
      return "INVALID_PARAMETER";
    case Status::BUSY:
      return "BUSY";
    case Status::INTERNAL_ERROR:
      return "INTERNAL_ERROR";
    case Status::ALIAS_LOOP:
      return "ALIAS_LOOP";
    case Status::CONNECTION_ERROR:
      return "CONNECTION_ERROR";
    default:
      return "UNKNOWN_STATUS";
  }
}
}  // namespace dsa
