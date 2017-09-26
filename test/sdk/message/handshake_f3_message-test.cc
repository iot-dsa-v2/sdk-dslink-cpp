#include "dsa/message.h"
#include "gtest/gtest.h"

using namespace dsa;

TEST(MessageTest, HandshakeF3__Constructor_01) {
  HandshakeF3Message message;

  uint16_t session_id_length = 64;
  std::string session_id(
      "session-id123456789012345678901234567890123456789012345678901234");
  message.session_id = session_id;

  uint16_t path_length = 32;
  std::string path("path5678901234567890123456789012");
  message.path = path;

  uint8_t auth[] = "auth5678901234567890123456789012";
  message.auth = std::vector<uint8_t>(auth, auth + Message::AUTH_LENGTH);

  // last_ack_id = 0xa1b2c3d4
  message.last_ack_id = 2712847316;

  message.allow_requester = true;

  message.size();

  uint8_t buf[1024];
  message.write(buf);

  // 15 + 1 + 2 + 64 + 4 + 2 + 32 + 32 = 152
  uint8_t expected_values[152];

  uint32_t message_size = 152;
  uint16_t header_size = StaticHeaders::TOTAL_SIZE;
  MessageType type = MessageType::HANDSHAKE3;
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

  uint8_t AllowRequesterOffset = StaticHeaders::TOTAL_SIZE;
  uint8_t SessionIdLengthOffset = AllowRequesterOffset + sizeof(bool);
  uint8_t SessionIdOffset = SessionIdLengthOffset + sizeof(uint16_t);
  uint8_t LastAckIdOffset = SessionIdOffset + message.session_id.size();
  uint8_t PathLengthOffset = LastAckIdOffset + sizeof(uint32_t);
  uint8_t PathOffset = PathLengthOffset + sizeof(uint16_t);
  uint8_t AuthOffset = PathOffset + path_length;

  std::memcpy(&expected_values[AllowRequesterOffset], &message.allow_requester,
              sizeof(bool));
  std::memcpy(&expected_values[SessionIdLengthOffset], &session_id_length,
              sizeof(session_id_length));
  std::memcpy(&expected_values[SessionIdOffset], message.session_id.data(),
              message.session_id.size());
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
