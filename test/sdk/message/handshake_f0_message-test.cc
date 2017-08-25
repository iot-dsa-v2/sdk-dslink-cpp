#include "dsa/message.h"
#include "gtest/gtest.h"

using namespace dsa;

class HandshakeF0MessageExt : public HandshakeF0Message {
 public:
  HandshakeF0MessageExt() : HandshakeF0Message() {}

  void update_static_header_ext() {
    HandshakeF0Message::update_static_header();
  }

  bool check_static_headers(uint8_t *expected_values, size_t size) {
    uint8_t buf[1024];
    static_headers.write(buf);

    return (memcmp(expected_values, buf, size) == 0);
  }
};

TEST(MessageTest, HandshakeF0__Constructor_01) {
  // public methods
  // SetRequestMessage();

  HandshakeF0MessageExt message;

  message.update_static_header_ext();

  uint8_t buf[1024];
  message.write(buf);
}