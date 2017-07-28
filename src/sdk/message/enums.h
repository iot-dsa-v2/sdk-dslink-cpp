#ifndef DSA_SDK_MESSAGE_ENUMS_H
#define DSA_SDK_MESSAGE_ENUMS_H

namespace dsa {

enum MessageType : uint8_t {
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

enum MessageStatus : uint8_t {
  // https://github.com/iot-dsa-v2/docs/blob/master/protocol/Status-Table.md

  StatusOk = 0x00,
  StatusInitializing = 0x01,
  StatusRefreshed = 0x08,
  StatusNotAvailable = 0x0E,
  StatusClosed = 0x10,
  StatusDisconnected = 0x1E,
  StatusPermissionDenied = 0x20,
  StatusInvalidMessage = 0x21,
  StatusInvalidParameter = 0x22,
  StatusBusy = 0x28,
  StatusAliasLoop = 0x41,
  StatusConnectionError = 0xF9,
};

}  // namespace dsa

#endif  // DSA_SDK_MESSAGE_ENUMS_H
