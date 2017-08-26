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

  message.update_static_header_ext();

  uint8_t buf[1024];
  message.write(buf);
}
