#include "dsa/message.h"
#include "dsa/util.h"
#include <gtest/gtest.h>

using namespace dsa;

bool check_static_headers(SubscribeRequestMessage& message,
                          uint8_t* expected_values, size_t size) {
  uint8_t buf[1024];
  message.write(buf);

  return (memcmp(expected_values, buf, size) == 0);
}

TEST(MessageTest, subscribe_request_message) {
  SubscribeRequestMessage subscribe_request;

  EXPECT_EQ(0, subscribe_request.get_qos());

  subscribe_request.set_qos(QosLevel::_2);
  EXPECT_EQ(QosLevel::_2, subscribe_request.get_qos());

  SubscribeOptions option = subscribe_request.get_subscribe_options();
  EXPECT_EQ(QosLevel::_2, option.qos);

  auto b = make_ref_<RefCountBytes>(256);

  EXPECT_EQ(17, subscribe_request.size());

  string_ path = "/a";

  subscribe_request.set_target_path(path);

  EXPECT_EQ(22, subscribe_request.size());

  subscribe_request.write(b->data());

  // parse a subscription message from the buffer
  SubscribeRequestMessage subscribe_request2(&b->front(), b->size());

  SubscribeOptions option2 = subscribe_request2.get_subscribe_options();
  EXPECT_EQ(QosLevel::_2, option2.qos);

  EXPECT_EQ(22, subscribe_request2.size());

  EXPECT_EQ(path, subscribe_request2.get_target_path().full_str());
}

TEST(MessageTest, SubscribeRequest__Constructor_01) {
  // public methods
  // SubscribeRequestMessage();

  SubscribeRequestMessage request;

  EXPECT_EQ(15, request.size());
  EXPECT_EQ(0, request.get_sequence_id());
  EXPECT_EQ(0, request.get_page_id());
  EXPECT_EQ(MessageType::SUBSCRIBE_REQUEST, request.type());
  EXPECT_TRUE(request.is_request());
  EXPECT_EQ(0, request.get_rid());

  EXPECT_FALSE(request.get_priority());
  EXPECT_EQ("", request.get_target_path().full_str());
  EXPECT_EQ("", request.get_permission_token());
  EXPECT_FALSE(request.get_no_stream());
  EXPECT_EQ(0, request.get_alias_count());
}

TEST(MessageTest, SubscribeRequest__Constructor_02) {
  //   SubscribeRequestMessage(const SubscribeRequestMessage&);

  const SubscribeRequestMessage src__request;
  SubscribeRequestMessage target__request(src__request);

  EXPECT_EQ(15, target__request.size());
  EXPECT_EQ(0, target__request.get_sequence_id());
  EXPECT_EQ(0, target__request.get_page_id());
  EXPECT_EQ(MessageType::SUBSCRIBE_REQUEST, target__request.type());
  EXPECT_TRUE(target__request.is_request());
  EXPECT_EQ(0, target__request.get_rid());

  EXPECT_FALSE(target__request.get_priority());
  EXPECT_EQ("", target__request.get_target_path().full_str());
  EXPECT_EQ("", target__request.get_permission_token());
  EXPECT_FALSE(target__request.get_no_stream());
  EXPECT_EQ(0, target__request.get_alias_count());

  string_ target_path("path/to/abc");
  target__request.set_target_path(target_path);

  EXPECT_EQ("", src__request.get_target_path().full_str());
  EXPECT_EQ(target_path, target__request.get_target_path().full_str());

  EXPECT_EQ(29, target__request.size());
}

TEST(MessageTest, SubscribeRequest__Constructor_03) {
  //   SubscribeRequestMessage(const uint8_t* data, size_t size);

  const uint8_t data[] = {0xf, 0x0, 0x0, 0x0, 0xf, 0x0, 0x1, 0x0,
                          0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
  size_t data_size = sizeof(data) / sizeof(uint8_t);

  SubscribeRequestMessage request(data, data_size);

  EXPECT_EQ(15, request.size());
  EXPECT_EQ(0, request.get_sequence_id());
  EXPECT_EQ(0, request.get_page_id());
  EXPECT_EQ(MessageType::SUBSCRIBE_REQUEST, request.type());
  EXPECT_TRUE(request.is_request());
  EXPECT_EQ(0, request.get_rid());

  EXPECT_FALSE(request.get_priority());
  EXPECT_EQ("", request.get_target_path().full_str());
  EXPECT_EQ("", request.get_permission_token());
  EXPECT_FALSE(request.get_no_stream());
  EXPECT_EQ(0, request.get_alias_count());
}

TEST(MessageTest, SubscribeRequest__Constructor_04) {
  //   SubscribeRequestMessage(const uint8_t* data, size_t size);

  SubscribeRequestMessage request;
  request.set_target_path("/request");
  SubscribeRequestMessage other = request;
  EXPECT_EQ("/request", other.get_target_path().full_str());
  other.set_target_path("/other");
  EXPECT_EQ("/request", request.get_target_path().full_str());
  EXPECT_EQ("/other", other.get_target_path().full_str());

  EXPECT_EQ(24, other.size());
  EXPECT_EQ(0, other.get_sequence_id());
  EXPECT_EQ(0, other.get_page_id());
  EXPECT_EQ(MessageType::SUBSCRIBE_REQUEST, other.type());
  EXPECT_TRUE(other.is_request());
  EXPECT_EQ(0, other.get_rid());

  EXPECT_FALSE(other.get_priority());
  EXPECT_EQ("/other", other.get_target_path().full_str());
  EXPECT_EQ("", other.get_permission_token());
  EXPECT_FALSE(other.get_no_stream());
  EXPECT_EQ(0, other.get_alias_count());
}

TEST(MessageTest, SubscribeRequest__get_subscribe_options) {
  //   SubscribeOptions get_subscribe_options() const;

  SubscribeRequestMessage request;
  SubscribeOptions options = request.get_subscribe_options();

  EXPECT_EQ(0, request.get_qos());

  request.set_qos(QosLevel::_2);
  EXPECT_EQ(QosLevel::_2, request.get_qos());

  SubscribeOptions option = request.get_subscribe_options();
  EXPECT_EQ(QosLevel::_2, option.qos);
}

TEST(MessageTest, SubscribeRequest__update_static_header) {
  // void update_static_header();
  SubscribeRequestMessage request;
  request.size();

  uint8_t expect_values[] = {0xf, 0x0, 0x0, 0x0, 0xf, 0x0};
  EXPECT_TRUE(check_static_headers(request, expect_values,
                                   sizeof(expect_values) / sizeof(uint8_t)));
}

TEST(MessageTest, SubscribeRequest__priority) {
  SubscribeRequestMessage request;

  EXPECT_FALSE(request.get_priority());
  request.set_priority(true);
  EXPECT_TRUE(request.get_priority());
}

TEST(MessageTest, SubscribeRequest__target_path) {
  SubscribeRequestMessage request;

  EXPECT_EQ("", request.get_target_path().full_str());
  request.set_target_path("path/to/node");
  EXPECT_EQ("path/to/node", request.get_target_path().full_str());
}

TEST(MessageTest, SubscribeRequest__permission_token) {
  // TODO: to be implemented
  SubscribeRequestMessage request;

  EXPECT_EQ("", request.get_permission_token());
  request.set_permission_token("permission-token");
  EXPECT_EQ("permission-token", request.get_permission_token());
}

TEST(MessageTest, SubscribeRequest__no_stream) {
  SubscribeRequestMessage request;

  EXPECT_FALSE(request.get_no_stream());
  request.set_no_stream(true);
  EXPECT_TRUE(request.get_no_stream());
}

TEST(MessageTest, SubscribeRequest__write) {
  SubscribeRequestMessage request;

  request.set_target_path("path/to/dsa");
  request.set_no_stream(true);

  request.size();

  uint8_t buf[1024];
  request.write(buf);

  uint8_t expected_values[] = {0x1e, 0x0,  0x0,  0x0,  0x1e, 0x0,  0x1,  0x0,
                               0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x80,
                               0x0b, 0x0,  0x70, 0x61, 0x74, 0x68, 0x2f, 0x74,
                               0x6f, 0x2f, 0x64, 0x73, 0x61, 0x11};

  EXPECT_EQ(0, memcmp(expected_values, buf,
                      sizeof(expected_values) / sizeof(uint8_t)));
}

TEST(MessageTest, SubscribeRequest__dynamic_structure) {
  SubscribeRequestMessage request;

  //    request.set_status(MessageStatus::CLOSED);
  request.set_sequence_id(1234);  // no effect
  request.set_page_id(4321);      // no effect
  request.set_alias_count(11);
  request.set_priority(true);
  request.set_no_stream(true);
  request.set_qos(QosLevel::_2);
  request.set_queue_size(5678);
  request.set_queue_time(1248);
  request.set_permission_token("ptoken");
  request.set_target_path("/target/path");

  request.size();

  uint8_t buf[1024];
  request.write(buf);

  uint8_t expected_values[] = {
      0x37, 0x0,  0x0,  0x0,  0x37, 0x0,  0x01, 0x0,  0x0,  0x0,  0x0,
      0x0,  0x0,  0x0,  0x0,  0x10, 0x08, 0x0b, 0x80, 0x0c, 0x0,  0x2f,
      0x74, 0x61, 0x72, 0x67, 0x65, 0x74, 0x2f, 0x70, 0x61, 0x74, 0x68,
      0x60, 0x06, 0x0,  0x70, 0x74, 0x6f, 0x6b, 0x65, 0x6e, 0x11, 0x12,
      0x02, 0x14, 0x2e, 0x16, 0x0,  0x0,  0x15, 0xe0, 0x04, 0x0,  0x0};

  EXPECT_EQ(0, memcmp(expected_values, buf,
                      sizeof(expected_values) / sizeof(uint8_t)));
}

TEST(MessageTest, SubscribeResponse__Constructor_01) {
  SubscribeResponseMessage response;

  EXPECT_EQ(15, response.size());
  EXPECT_EQ(0, response.get_sequence_id());
  EXPECT_EQ(0, response.get_page_id());
  EXPECT_EQ(MessageType::SUBSCRIBE_RESPONSE, response.type());
  EXPECT_FALSE(response.is_request());
  EXPECT_EQ(0, response.get_rid());
  EXPECT_EQ(MessageType::SUBSCRIBE_RESPONSE,
            response.get_response_type(MessageType::SUBSCRIBE_REQUEST));
}

TEST(MessageTest, SubscribeResponse__Constructor_02) {
  SubscribeResponseMessage source_response;

  source_response.set_status(MessageStatus::CLOSED);
  source_response.set_sequence_id(1234);
  source_response.set_page_id(4321);
  source_response.set_source_path("/source/path");

  source_response.size();

  uint8_t src_buf[1024];
  source_response.write(src_buf);

  //
  size_t buf_size = 42;
  SubscribeResponseMessage response(src_buf, buf_size);

  response.size();

  uint8_t buf[1024];
  response.write(buf);

  EXPECT_EQ(0, memcmp(src_buf, buf, buf_size));
}

TEST(MessageTest, SubscribeResponse__source_path) {
  SubscribeResponseMessage response;

  EXPECT_EQ("", response.get_source_path());
  response.set_source_path("/source/path");
  EXPECT_EQ("/source/path", response.get_source_path());
}

TEST(MessageTest, SubscribeResponse__status) {
  SubscribeResponseMessage response;

  static const MessageStatus message_status_all[]{
      MessageStatus::OK,
      MessageStatus::INITIALIZING,
      MessageStatus::REFRESHED,
      MessageStatus::NOT_AVAILABLE,
      MessageStatus::CLOSED,
      MessageStatus::DISCONNECTED,
      MessageStatus::PERMISSION_DENIED,
      MessageStatus::INVALID_MESSAGE,
      MessageStatus::INVALID_PARAMETER,
      MessageStatus::BUSY,
      MessageStatus::ALIAS_LOOP,
      MessageStatus::CONNECTION_ERROR,
  };
  for (const auto status : message_status_all) {
    response.set_status(status);
    EXPECT_EQ(status, response.get_status());
  }
}

TEST(MessageTest, SubscribeResponse__write) {
  SubscribeResponseMessage response;

  response.set_source_path("source/path");
  response.set_status(MessageStatus::BUSY);

  response.size();

  uint8_t buf[1024];
  response.write(buf);

  uint8_t expected_values[] = {0x1f, 0x0,  0x0,  0x0,  0x1f, 0x0,  0x81, 0x0,
                               0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
                               0x48, 0x81, 0x0b, 0x0,  0x73, 0x6f, 0x75, 0x72,
                               0x63, 0x65, 0x2f, 0x70, 0x61, 0x74, 0x68};

  EXPECT_EQ(0, memcmp(expected_values, buf,
                      sizeof(expected_values) / sizeof(uint8_t)));
}

TEST(MessageTest, SubscribeResponse__dynamic_structure) {
  SubscribeResponseMessage response;

  response.set_status(MessageStatus::CLOSED);
  response.set_sequence_id(1234);
  response.set_page_id(4321);
  response.set_source_path("/source/path");

  response.size();

  uint8_t buf[1024];
  response.write(buf);

  uint8_t expected_values[] = {
      0x2a, 0x0,  0x0,  0x0,  0x2a, 0x0,  0x81, 0x0,  0x0,  0x0,  0x0,
      0x0,  0x0,  0x0,  0x0,  0x0,  0x20, 0x01, 0xd2, 0x04, 0x0,  0x0,
      0x02, 0xe1, 0x10, 0x0,  0x0,  0x81, 0x0c, 0x0,  0x2f, 0x73, 0x6f,
      0x75, 0x72, 0x63, 0x65, 0x2f, 0x70, 0x61, 0x74, 0x68};

  EXPECT_EQ(0, memcmp(expected_values, buf,
                      sizeof(expected_values) / sizeof(uint8_t)));
}

TEST(MessageTest, SubscribeResponse__copy) {
  SubscribeResponseMessage response;

  response.set_status(MessageStatus::CLOSED);
  response.set_sequence_id(1234);
  response.set_page_id(4321);
  response.set_source_path("/source/path");

  response.size();

  SubscribeResponseMessage dup_response(response);

  uint8_t buf[1024];
  dup_response.write(buf);

  uint8_t expected_values[] = {
      0x2a, 0x0,  0x0,  0x0,  0x2a, 0x0,  0x81, 0x0,  0x0,  0x0,  0x0,
      0x0,  0x0,  0x0,  0x0,  0x0,  0x20, 0x01, 0xd2, 0x04, 0x0,  0x0,
      0x02, 0xe1, 0x10, 0x0,  0x0,  0x81, 0x0c, 0x0,  0x2f, 0x73, 0x6f,
      0x75, 0x72, 0x63, 0x65, 0x2f, 0x70, 0x61, 0x74, 0x68};

  EXPECT_EQ(0, memcmp(expected_values, buf,
                      sizeof(expected_values) / sizeof(uint8_t)));
}
