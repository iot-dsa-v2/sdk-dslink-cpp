#include "dsa/message.h"
#include "gtest/gtest.h"

using namespace dsa;

TEST(MessageTest, subscribe_request_message) {
  SubscribeRequestMessage subscribe_request;

  EXPECT_EQ(0, subscribe_request.get_qos());

  subscribe_request.set_qos(StreamQos::_2);
  EXPECT_EQ(StreamQos::_2, subscribe_request.get_qos());

  SubscribeOptions option = subscribe_request.get_subscribe_option();
  EXPECT_EQ(StreamQos::_2, option.qos);

  std::shared_ptr<Buffer> b = std::make_shared<Buffer>(256);

  EXPECT_EQ(17, subscribe_request.size());

  std::string path = "/a";

  subscribe_request.set_target_path(path);

  EXPECT_EQ(22, subscribe_request.size());

  subscribe_request.write(b->data());

  SharedBuffer sb = b->get_shared_buffer(0, subscribe_request.size());

  // parse a subscription message from the buffer
  SubscribeRequestMessage subscribe_request2(sb);

  SubscribeOptions option2 = subscribe_request2.get_subscribe_option();
  EXPECT_EQ(StreamQos::_2, option2.qos);

  EXPECT_EQ(22, subscribe_request2.size());

  EXPECT_EQ(path, subscribe_request2.get_target_path());
}


