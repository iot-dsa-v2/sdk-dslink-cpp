#ifndef DSA_SDK_MESSAGE_ENUMS_H
#define DSA_SDK_MESSAGE_ENUMS_H

namespace dsa {

enum class MessageType : uint8_t {
  // https://github.com/iot-dsa-v2/docs/blob/master/protocol/Header-Structure.md

  SubscribeRequest = 0x01,
  ListRequest = 0x02,
  InvokeRequest = 0x03,
  SetRequest = 0x04,
  SubscribeResponse = 0x81,
  ListResponse = 0x82,
  InvokeResponse = 0x83,
  SetResponse = 0x84,
  Handshake0 = 0xF0,
  Handshake1 = 0xF1,
  Handshake2 = 0xF2,
  Handshake3 = 0xF3
};

enum class MessageStatus : uint8_t {
  // https://github.com/iot-dsa-v2/docs/blob/master/protocol/Status-Table.md

  Ok = 0x00,
  Initializing = 0x01,
  Refreshed = 0x08,
  NotAvailable = 0x0E,
  Closed = 0x10,
  Disconnected = 0x1E,
  PermissionDenied = 0x20,
  InvalidMessage = 0x21,
  InvalidParameter = 0x22,
  Busy = 0x28,
  AliasLoop = 0x41,
  ConnectionError = 0xF9,
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
