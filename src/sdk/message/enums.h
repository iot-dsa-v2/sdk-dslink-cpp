#ifndef DSA_SDK_MESSAGE_ENUMS_H
#define DSA_SDK_MESSAGE_ENUMS_H

namespace dsa {

enum class MessageType : uint8_t {
  // https://github.com/iot-dsa-v2/docs/blob/master/protocol/Header-Structure.md

  SUBSCRIBE_REQUEST = 0x01,
  LIST_REQUEST = 0x02,
  INVOKE_REQUEST = 0x03,
  SET_REQUEST = 0x04,
  SUBSCRIBE_RESPONSE = 0x81,
  LIST_RESPONSE = 0x82,
  INVOKE_RESPONSE = 0x83,
  SET_RESPONSE = 0x84,
  HANDSHAKE0 = 0xF0,
  HANDSHAKE1 = 0xF1,
  HANDSHAKE2 = 0xF2,
  HANDSHAKE3 = 0xF3
};

enum class MessageStatus : uint8_t {
  // https://github.com/iot-dsa-v2/docs/blob/master/protocol/Status-Table.md

  OK = 0x00,
  INITIALIZING = 0x01,
  REFRESHED = 0x08,
  NOT_AVAILABLE = 0x0E,
  CLOSED = 0x10,
  DISCONNECTED = 0x1E,
  PERMISSION_DENIED = 0x20,
  INVALID_MESSAGE = 0x21,
  INVALID_PARAMETER = 0x22,
  BUSY = 0x28,
  ALIAS_LOOP = 0x41,
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

}  // namespace dsa

#endif  // DSA_SDK_MESSAGE_ENUMS_H
