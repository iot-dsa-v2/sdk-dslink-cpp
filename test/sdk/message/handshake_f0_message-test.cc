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

  //  EXPECT_EQ(15, request.size());
  //  EXPECT_EQ(0, request.get_sequence_id());
  //  EXPECT_EQ(0, request.get_page_id());
  //  EXPECT_EQ(MessageType::SetRequest, request.type());
  //  EXPECT_EQ(true, request.is_request());
  //  EXPECT_EQ(0, request.request_id());
  //
  //  EXPECT_EQ(false, request.get_priority());
  //  EXPECT_EQ("", request.get_target_path());
  //  EXPECT_EQ("", request.get_permission_token());
  //  EXPECT_EQ(false, request.get_no_stream());
  //  EXPECT_EQ(0, request.get_alias_count());

  uint8_t buf[1024];
  message.write(buf);

int i;
}