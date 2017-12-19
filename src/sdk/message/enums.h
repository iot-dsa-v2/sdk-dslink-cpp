#ifndef DSA_SDK_MESSAGE_ENUMS_H
#define DSA_SDK_MESSAGE_ENUMS_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <iosfwd>

namespace dsa {

enum class MessageType : uint8_t {
  // https://github.com/iot-dsa-v2/docs/blob/master/protocol/Header-Structure.md
  SUBSCRIBE_REQUEST = 0x01,
  LIST_REQUEST = 0x02,
  INVOKE_REQUEST = 0x03,
  SET_REQUEST = 0x04,
  
  CLOSE_REQUEST = 0x0F,

  SUBSCRIBE_RESPONSE = 0x81,
  LIST_RESPONSE = 0x82,
  INVOKE_RESPONSE = 0x83,
  SET_RESPONSE = 0x84,

  HANDSHAKE0 = 0xF0,
  HANDSHAKE1 = 0xF1,
  HANDSHAKE2 = 0xF2,
  HANDSHAKE3 = 0xF3,

  ACK = 0xF8,
  PING = 0xF9,
  INVALID = 0xFF
};
std::ostream &operator<<(std::ostream &os, dsa::MessageType type);

enum class MessageStatus : uint8_t {
  // https://github.com/iot-dsa-v2/docs/blob/master/protocol/Status-Table.md

  OK = 0x00,
  INITIALIZING = 0x01,
  REFRESHED = 0x02,
  REFRESHED_INITIALIZING = 0x03,
  NOT_AVAILABLE = 0x0E,
  CLOSED = 0x20,
  DISCONNECTED = 0x2E,
  PERMISSION_DENIED = 0x40,
  NOT_SUPPORTED = 0x41,
  INVALID_MESSAGE = 0x44,
  INVALID_PARAMETER = 0x45,
  BUSY = 0x48,
  ALIAS_LOOP = 0x61,
  CONNECTION_ERROR = 0xF9,
};

enum class PermissionLevel : uint8_t {
  NONE = 0x00,
  LIST = 0x10,
  READ = 0x20,
  WRITE = 0x30,
  CONFIG = 0x40,
  NEVER = 0x7f,
  UNKNOWN = 0xff
};

const char* to_string(PermissionLevel permission_level);



}  // namespace dsa

#endif  // DSA_SDK_MESSAGE_ENUMS_H
