#include "dsa/message.h"
#include "gtest/gtest.h"

using namespace dsa;

class HandshakeF3MessageExt : public HandshakeF3Message {
 public:
  HandshakeF3MessageExt() : HandshakeF3Message() {}

  void update_static_header_ext() {
    HandshakeF3Message::update_static_header();
  }

  bool check_static_headers(uint8_t *expected_values, size_t size) {
    uint8_t buf[1024];
    static_headers.write(buf);

    return (memcmp(expected_values, buf, size) == 0);
  }
};

TEST(MessageTest, HandshakeF3__Constructor_01) {
  HandshakeF3MessageExt message;

  uint16_t session_id_length = 64;
  std::string session_id("session-id123456789012345678901234567890123456789012345678901234");
  message.session_id = session_id;

  uint16_t path_length = 32;
  std::string path("path5678901234567890123456789012");
  message.path = path;

  uint8_t auth[] = "auth5678901234567890123456789012";
  message.other_auth = std::vector<uint8_t>(auth, auth + Message::AuthLength);

  message.update_static_header_ext();

  uint8_t buf[1024];
  message.write(buf);

  // 15 + 2 + 64 + 2 + 32 + 32 = 147
  uint8_t expected_values[147];

  uint32_t message_size = 147;
  uint16_t header_size = StaticHeaders::TotalSize;
  MessageType type = MessageType::Handshake3;
  uint32_t request_id = 0;
  uint32_t ack_id = 0;

  std::memcpy(&expected_values[StaticHeaders::MessageSizeOffset], &message_size,
              sizeof(uint32_t));
  std::memcpy(&expected_values[StaticHeaders::HeaderSizeOffset], &header_size,
              sizeof(uint16_t));
  std::memcpy(&expected_values[StaticHeaders::TypeOffset], &type,
              sizeof(uint8_t));
  std::memcpy(&expected_values[StaticHeaders::RequestIdOffset], &request_id,
              sizeof(request_id));
  std::memcpy(&expected_values[StaticHeaders::AckIdOffset], &ack_id,
              sizeof(ack_id));

  uint8_t SessionIdLengthOffset = StaticHeaders::TotalSize;
  uint8_t SessionIdOffset = SessionIdLengthOffset + sizeof(uint16_t);
  uint8_t PathLengthOffset = SessionIdOffset + message.session_id.size();
  uint8_t PathOffset = PathLengthOffset + sizeof(uint16_t);
  uint8_t AuthOffset = PathOffset + path_length;

  std::memcpy(&expected_values[SessionIdLengthOffset], &session_id_length,  sizeof(session_id_length));
  std::memcpy(&expected_values[SessionIdOffset], message.session_id.data(), message.session_id.size());
  std::memcpy(&expected_values[PathLengthOffset], &path_length,  sizeof(path_length));
  std::memcpy(&expected_values[PathOffset], message.path.data(), message.path.size());
  std::memcpy(&expected_values[AuthOffset], message.other_auth.data(), Message::AuthLength);

  EXPECT_EQ(0, memcmp(expected_values, buf, message_size));

}
