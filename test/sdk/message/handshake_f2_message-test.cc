#include "dsa/message.h"
#include "gtest/gtest.h"

using namespace dsa;

TEST(MessageTest, HandshakeF2__Constructor_01) {
  HandshakeF2Message message;

  string_ token(
      "token6789012345678901234567890123456789012345678901234567890123456789012"
      "34567890123456789012345678901234567890123456789012345678");
  message.token = token;
  message.is_responder = false;
  string_ previous_session_id(
      "session-id123456789012345678901234567890123456789012345678901234");
  message.previous_session_id = previous_session_id;
  uint8_t auth[] = "auth5678901234567890123456789012";
  message.auth = std::vector<uint8_t>(auth, auth + Message::AUTH_LENGTH);

  // last_ack_id = 0xa1b2c3d4
  message.last_ack_id = 2712847316;

  uint16_t path_length = 0;


  message.size();

  uint8_t buf[1024];
  message.write(buf);

  // 15 + 2 + 128 + 1 + 2 + 64 + 4 + 32 + 2 = 250
  uint8_t expected_values[250];

  uint32_t message_size = 250;
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
  uint8_t IsResponderOffset = TokenOffset + token_length;
  uint8_t PreviousSessionIdLengthOffset = IsResponderOffset + sizeof(bool);
  uint8_t PreviousSessionIdOffset =
      PreviousSessionIdLengthOffset + sizeof(session_id_length);
  uint8_t LastAckIdOffset = PreviousSessionIdOffset + session_id_length;

  uint8_t PathLengthOffset = LastAckIdOffset + sizeof(uint32_t);
  uint8_t PathOffset = PathLengthOffset + sizeof(uint16_t);
  uint8_t AuthOffset = PathOffset + path_length;

  std::memcpy(&expected_values[TokenLengthOffset], &token_length,
              sizeof(token_length));
  std::memcpy(&expected_values[TokenOffset], message.token.data(),
              message.token.size());
  std::memcpy(&expected_values[IsResponderOffset], &message.is_responder,
              sizeof(bool));
  std::memcpy(&expected_values[PreviousSessionIdLengthOffset],
              &session_id_length, sizeof(session_id_length));
  std::memcpy(&expected_values[PreviousSessionIdOffset],
              message.previous_session_id.data(),
              message.previous_session_id.size());
  std::memcpy(&expected_values[LastAckIdOffset], &message.last_ack_id,
              sizeof(uint32_t));
  std::memcpy(&expected_values[PathLengthOffset], &path_length,
              sizeof(path_length));
  std::memcpy(&expected_values[PathOffset], message.path.data(),
              message.path.size());
  std::memcpy(&expected_values[AuthOffset], message.auth.data(),
              Message::AUTH_LENGTH);

  EXPECT_EQ(0, memcmp(expected_values, buf, message_size));
}

TEST(MessageTest, HandshakeF2__get_response_type) {
  HandshakeF2Message message;

  EXPECT_EQ(MessageType::INVALID, message.get_response_type(MessageType::HANDSHAKE2));
}
