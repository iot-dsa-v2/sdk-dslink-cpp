#include "dsa/message.h"
#include "gtest/gtest.h"

using namespace dsa;

TEST(MessageTest, HandshakeF2__Constructor_01) {
  HandshakeF2Message message;

  std::string token(
      "token6789012345678901234567890123456789012345678901234567890123456789012"
      "34567890123456789012345678901234567890123456789012345678");
  message.token = token;
  message.is_requester = true;
  message.is_responder = false;
  std::string previous_session_id(
      "session-id123456789012345678901234567890123456789012345678901234");
  message.previous_session_id = previous_session_id;
  uint8_t auth[] = "auth5678901234567890123456789012";
  message.auth = std::vector<uint8_t>(auth, auth + Message::AUTH_LENGTH);

  message.size();

  uint8_t buf[1024];
  message.write(buf);

  // 15 + 2 + 128 + 1 + 1 + 2 + 64 + 32 = 245
  uint8_t expected_values[245];

  uint32_t message_size = 245;
  uint16_t header_size = StaticHeaders::TOTAL_SIZE;
  MessageType type = MessageType::HANDSHAKE2;
  uint32_t request_id = 0;
  uint32_t ack_id = 0;

  std::memcpy(&expected_values[StaticHeaders::MESSAGE_SIZE_OFFSET],
              &message_size, sizeof(uint32_t));
  std::memcpy(&expected_values[StaticHeaders::HEADER_SIZE_OFFSET], &header_size,
              sizeof(uint16_t));
  std::memcpy(&expected_values[StaticHeaders::TYPE_OFFSET], &type,
              sizeof(uint8_t));
  std::memcpy(&expected_values[StaticHeaders::REQUEST_ID_OFFSET], &request_id,
              sizeof(request_id));
  std::memcpy(&expected_values[StaticHeaders::ACK_ID_OFFSET], &ack_id,
              sizeof(ack_id));

  uint16_t token_length = 128;
  uint16_t session_id_length = 64;
  uint8_t TokenLengthOffset = StaticHeaders::TOTAL_SIZE;
  uint8_t TokenOffset = TokenLengthOffset + sizeof(token_length);
  uint8_t IsRequesterOffset = TokenOffset + token_length;
  uint8_t IsResponderOffset = IsRequesterOffset + sizeof(bool);
  uint8_t SessionIdLengthOffset = IsResponderOffset + sizeof(bool);
  uint8_t SessionIdOffset = SessionIdLengthOffset + sizeof(uint16_t);
  uint8_t AuthOffset = SessionIdOffset + session_id_length;

  std::memcpy(&expected_values[TokenLengthOffset], &token_length,
              sizeof(token_length));
  std::memcpy(&expected_values[TokenOffset], message.token.data(),
              message.token.size());
  std::memcpy(&expected_values[IsRequesterOffset], &message.is_requester,
              sizeof(bool));
  std::memcpy(&expected_values[IsResponderOffset], &message.is_responder,
              sizeof(bool));
  std::memcpy(&expected_values[SessionIdLengthOffset], &session_id_length,
              sizeof(session_id_length));
  std::memcpy(&expected_values[SessionIdOffset], message.previous_session_id.data(),
              message.previous_session_id.size());
  std::memcpy(&expected_values[AuthOffset], message.auth.data(),
              Message::AUTH_LENGTH);

  EXPECT_EQ(0, memcmp(expected_values, buf, message_size));
}
