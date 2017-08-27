#include "dsa/message.h"
#include "gtest/gtest.h"

#include "util/little_endian.h"

using namespace dsa;

class HandshakeF1MessageExt : public HandshakeF1Message {
 public:
  HandshakeF1MessageExt() : HandshakeF1Message() {}

  void update_static_header_ext() {
    HandshakeF1Message::update_static_header();
  }

  bool check_static_headers(uint8_t *expected_values, size_t size) {
    uint8_t buf[1024];
    static_headers.write(buf);

    return (memcmp(expected_values, buf, size) == 0);
  }
};

TEST(MessageTest, HandshakeF1__Constructor_01) {
  HandshakeF1MessageExt message;

  message.dsid = "dsid";

  uint8_t public_key[] =
      "public-key1234567890123456789012345678901234567890123456789012345";
  message.public_key =
      std::vector<uint8_t>(public_key, public_key + Message::PublicKeyLength);

  uint8_t salt[] = "salt5678901234567890123456789012";
  message.salt = std::vector<uint8_t>(salt, salt + Message::SaltLength);

  message.update_static_header_ext();

  uint8_t buf[1024];
  message.write(buf);

  // 15 + 1 + 4 + 65 + 32 = 117
  uint8_t expected_values[117];

  uint32_t message_size = 117;
  uint16_t header_size = message_size;
  MessageType type = MessageType::Handshake1;
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

  uint8_t DsidLengthOffset = StaticHeaders::TotalSize;
  uint8_t DsidOffset = DsidLengthOffset + sizeof(uint16_t);
  uint8_t PublicKeyOffset = DsidOffset + message.dsid.size();
  uint8_t SaltOffset = PublicKeyOffset + Message::PublicKeyLength;

  write_16_t(&expected_values[DsidLengthOffset], message.dsid.length());

  std::memcpy(&expected_values[DsidOffset], message.dsid.data(),
              message.dsid.size());
  std::memcpy(&expected_values[PublicKeyOffset], message.public_key.data(),
              Message::PublicKeyLength);
  std::memcpy(&expected_values[SaltOffset], message.salt.data(),
              message.salt.size());

  EXPECT_EQ(0, memcmp(expected_values, buf, message_size));
}
