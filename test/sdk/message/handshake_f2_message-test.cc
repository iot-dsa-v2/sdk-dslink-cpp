#include "dsa/message.h"
#include "gtest/gtest.h"

using namespace dsa;

class HandshakeF2MessageExt : public HandshakeF2Message {
 public:
  HandshakeF2MessageExt() : HandshakeF2Message() {}

  void update_static_header_ext() {
    HandshakeF2Message::update_static_header();
  }

  bool check_static_headers(uint8_t *expected_values, size_t size) {
    uint8_t buf[1024];
    static_headers.write(buf);

    return (memcmp(expected_values, buf, size) == 0);
  }
};

TEST(MessageTest, HandshakeF2__Constructor_01) {
  HandshakeF2MessageExt message;

  message.token_length = 128;
  std::string token("token678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678");
  message.token = token;
  message.is_requester = true;
  message.is_responder = false;
  message.session_id_length = 64;
  std::string session_id("session-id123456789012345678901234567890123456789012345678901234");
  message.session_id = session_id;
  uint8_t auth[] = "auth5678901234567890123456789012";
  message.auth = std::vector<uint8_t>(auth, auth + Message::AuthLength);

  message.update_static_header_ext();

  uint8_t buf[1024];
  message.write(buf);

  // 15 + 2 + 128 + 1 + 1 + 2 + 64 + 32 = 245
  uint8_t expected_values[245];

  uint32_t message_size = 245;
  uint16_t header_size = message_size;
  MessageType type = MessageType::Handshake2;
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

  uint8_t TokenLengthOffset = StaticHeaders::TotalSize;
  uint8_t TokenOffset = TokenLengthOffset + sizeof(message.token_length);
  uint8_t IsRequesterOffset = TokenOffset + message.token_length;
  uint8_t IsResponderOffset = IsRequesterOffset + sizeof(bool);
  uint8_t SessionIdLengthOffset = IsResponderOffset + sizeof(bool);
  uint8_t SessionIdOffset = SessionIdLengthOffset + sizeof(uint16_t);
  uint8_t AuthOffset = SessionIdOffset + message.session_id_length;

  std::memcpy(&expected_values[TokenLengthOffset], &message.token_length,  sizeof(message.token_length));
  std::memcpy(&expected_values[TokenOffset], message.token.data(), message.token.size());
  std::memcpy(&expected_values[IsRequesterOffset], &message.is_requester, sizeof(bool));
  std::memcpy(&expected_values[IsResponderOffset], &message.is_responder, sizeof(bool));
  std::memcpy(&expected_values[SessionIdLengthOffset], &message.session_id_length,  sizeof(message.session_id_length));
  std::memcpy(&expected_values[SessionIdOffset], message.session_id.data(), message.session_id.size());
  std::memcpy(&expected_values[AuthOffset], message.auth.data(), Message::AuthLength);

//  EXPECT_EQ(0, memcmp(expected_values, buf, message_size));
}
