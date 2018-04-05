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

  // invalid message types
  // these message types is not allowed in network
  PAGED = 0xFE,
  INVALID = 0xFF
};
std::ostream& operator<<(std::ostream& os, dsa::MessageType type);

enum class Status : uint8_t {
  // https://github.com/iot-dsa-v2/docs/blob/master/protocol/Status-Table.md

  OK = 0x00,
  INITIALIZING = 0x01,
  REFRESHED = 0x02,
  NOT_AVAILABLE = 0x0E,
  DROPPED = 0x10,
  DONE = 0x20,
  DISCONNECTED = 0x2E,
  PERMISSION_DENIED = 0x40,
  NOT_SUPPORTED = 0x41,
  INVALID_MESSAGE = 0x44,
  INVALID_PARAMETER = 0x45,
  BUSY = 0x48,
  INTERNAL_ERROR = 0x50,
  ALIAS_LOOP = 0x61,
  CONNECTION_ERROR = 0xC9,

  // internal usage, never sent in dsa protocol

  // used by VarBytes, unclear status of var
  UNDEFINED = 0xF0,
  // used by VarBytes, value will encode to empty bytes
  BLANK = 0xF1,

  // general error
  GENERAL_ERROR = 0xFF
};

struct StatusDetail {
  Status code;
  string_ detail;
  StatusDetail(Status code = Status::DONE, const string_& detail = "")
      : code(code), detail(detail) {}
  bool is_done() { return code == Status::DONE; }
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

class PermissionName {
 public:
  static constexpr const char* NONE = "none";
  static constexpr const char* LIST = "list";
  static constexpr const char* READ = "read";
  static constexpr const char* WRITE = "write";
  static constexpr const char* CONFIG = "config";
  static constexpr const char* NEVER = "never";

  static const char* convert(PermissionLevel level);
  static PermissionLevel parse(string_ str);
};

enum class MergeQueueResult { NORMAL, SKIP_THIS, SKIP_NEXT };

const char* to_string(PermissionLevel permission_level);

const char* to_string(Status status);

}  // namespace dsa

#endif  // DSA_SDK_MESSAGE_ENUMS_H
