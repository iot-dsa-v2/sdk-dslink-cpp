#include "dsa/message.h"
#include "gtest/gtest.h"

#include "util/little_endian.h"

using namespace dsa;

TEST(MessageTest, HandshakeF1__Constructor_01) {
  HandshakeF1Message message;

  message.dsid = "dsid";

  uint8_t public_key[] =
      "public-key1234567890123456789012345678901234567890123456789012345";
  message.public_key =
      std::vector<uint8_t>(public_key, public_key + Message::PUBLIC_KEY_LENGTH);

  uint8_t salt[] = "salt5678901234567890123456789012";
  message.salt = std::vector<uint8_t>(salt, salt + Message::SALT_LENGTH);

  message.size();

  uint8_t buf[1024];
  message.write(buf);

  uint8_t expected_values[110];

  uint32_t message_size = 110;
  uint16_t header_size = StaticHeaders::SHORT_TOTAL_SIZE;
  MessageType type = MessageType::HANDSHAKE1;
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

  uint8_t DsidLengthOffset = StaticHeaders::SHORT_TOTAL_SIZE;
  uint8_t DsidOffset = DsidLengthOffset + sizeof(uint16_t);
  uint8_t PublicKeyOffset = DsidOffset + message.dsid.size();
  uint8_t SaltOffset = PublicKeyOffset + Message::PUBLIC_KEY_LENGTH;

  write_16_t(&expected_values[DsidLengthOffset], message.dsid.length());

  std::memcpy(&expected_values[DsidOffset], message.dsid.data(),
              message.dsid.size());
  std::memcpy(&expected_values[PublicKeyOffset], message.public_key.data(),
              Message::PUBLIC_KEY_LENGTH);
  std::memcpy(&expected_values[SaltOffset], message.salt.data(),
              message.salt.size());

  EXPECT_EQ(0, memcmp(expected_values, buf, message_size));
}

TEST(MessageTest, HandshakeF1__get_response_type) {
  HandshakeF1Message message;

  EXPECT_EQ(MessageType::INVALID, message.get_response_type(MessageType::HANDSHAKE1));
}

