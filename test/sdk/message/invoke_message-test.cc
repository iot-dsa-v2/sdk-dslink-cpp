#include "dsa/message.h"
#include "gtest/gtest.h"

using namespace dsa;

class InvokeRequestMessageExt : public InvokeRequestMessage {
 public:
  InvokeRequestMessageExt() : InvokeRequestMessage() {}

  bool check_static_headers(uint8_t *expected_values, size_t size) {
    uint8_t buf[1024];
    static_headers.write(buf);

    return (memcmp(expected_values, buf, size) == 0);
  }
};

class InvokeResponseMessageExt : public InvokeResponseMessage {
 public:
  InvokeResponseMessageExt() : InvokeResponseMessage() {}

  bool check_static_headers(uint8_t *expected_values, size_t size) {
    uint8_t buf[1024];
    static_headers.write(buf);

    return (memcmp(expected_values, buf, size) == 0);
  }
};

TEST(MessageTest, InvokeRequest__Constructor_01) {
  // public methods
  // InvokeRequestMessage();

  InvokeRequestMessage request;

  EXPECT_EQ(15, request.size());
  EXPECT_EQ(0, request.get_sequence_id());
  EXPECT_EQ(0, request.get_page_id());
  EXPECT_EQ(MessageType::InvokeRequest, request.type());
  EXPECT_EQ(true, request.is_request());
  EXPECT_EQ(0, request.request_id());

  EXPECT_EQ(false, request.get_priority());
  EXPECT_EQ("", request.get_target_path());
  EXPECT_EQ("", request.get_permission_token());
  EXPECT_EQ(false, request.get_no_stream());
  EXPECT_EQ(0, request.get_alias_count());
}

TEST(MessageTest, InvokeRequest__Constructor_02) {
  //   InvokeRequestMessage(const InvokeRequestMessage&);

  const InvokeRequestMessage src__request;
  InvokeRequestMessage target__request(src__request);

  EXPECT_EQ(15, target__request.size());
  EXPECT_EQ(0, target__request.get_sequence_id());
  EXPECT_EQ(0, target__request.get_page_id());
  EXPECT_EQ(MessageType::InvokeRequest, target__request.type());
  EXPECT_EQ(true, target__request.is_request());
  EXPECT_EQ(0, target__request.request_id());

  EXPECT_EQ(false, target__request.get_priority());
  EXPECT_EQ("", target__request.get_target_path());
  EXPECT_EQ("", target__request.get_permission_token());
  EXPECT_EQ(false, target__request.get_no_stream());
  EXPECT_EQ(0, target__request.get_alias_count());

  std::string target_path("path/to/abc");
  target__request.set_target_path(target_path);

  EXPECT_EQ("", src__request.get_target_path());
  EXPECT_EQ(target_path, target__request.get_target_path());
}

TEST(MessageTest, InvokeRequest__Constructor_03) {
  //   InvokeRequestMessage(const uint8_t* data, size_t size);

  const uint8_t data[] = {0xf, 0x0, 0x0, 0x0, 0xf, 0x0, 0x3, 0x0,
                          0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
  size_t data_size = sizeof(data) / sizeof(uint8_t);

  InvokeRequestMessage request(data, data_size);

  EXPECT_EQ(15, request.size());
  EXPECT_EQ(0, request.get_sequence_id());
  EXPECT_EQ(0, request.get_page_id());
  EXPECT_EQ(MessageType::InvokeRequest, request.type());
  EXPECT_EQ(true, request.is_request());
  EXPECT_EQ(0, request.request_id());

  EXPECT_EQ(false, request.get_priority());
  EXPECT_EQ("", request.get_target_path());
  EXPECT_EQ("", request.get_permission_token());
  EXPECT_EQ(false, request.get_no_stream());
  EXPECT_EQ(0, request.get_alias_count());

  uint8_t buf[1024];
  request.size();
  request.write(buf);
}

TEST(MessageTest, InvokeRequest__Constructor_04) {
  //   InvokeRequestMessage(const uint8_t* data, size_t size);

  InvokeRequestMessage request;
  request.set_target_path("/request");
  InvokeRequestMessage other = request;
  EXPECT_EQ("/request", other.get_target_path());
  other.set_target_path("/other");
  EXPECT_EQ("/request", request.get_target_path());
  EXPECT_EQ("/other", other.get_target_path());

  EXPECT_EQ(24, other.size());
  EXPECT_EQ(0, other.get_sequence_id());
  EXPECT_EQ(0, other.get_page_id());
  EXPECT_EQ(MessageType::InvokeRequest, other.type());
  EXPECT_EQ(true, other.is_request());
  EXPECT_EQ(0, other.request_id());

  EXPECT_EQ(false, other.get_priority());
  EXPECT_EQ("/other", other.get_target_path());
  EXPECT_EQ("", other.get_permission_token());
  EXPECT_EQ(false, other.get_no_stream());
  EXPECT_EQ(0, other.get_alias_count());
}

TEST(MessageTest, InvokeRequest__get_set_options) {
  //   InvokeOptions get_invoke_options() const;

  InvokeRequestMessage request;
  InvokeOptions option = request.get_invoke_options();

  EXPECT_EQ(1, sizeof(option));
}

TEST(MessageTest, InvokeRequest__update_static_header) {
  // void update_static_header();
  InvokeRequestMessageExt request;
  request.size();

  uint8_t expect_values[] = {0xf, 0x0, 0x0, 0x0, 0xf, 0x0};
  EXPECT_EQ(true,
            request.check_static_headers(
                expect_values, sizeof(expect_values) / sizeof(uint8_t)));
}

TEST(MessageTest, InvokeRequest__priority) {
  InvokeRequestMessage request;

  EXPECT_EQ(false, request.get_priority());
  request.set_priority(true);
  EXPECT_EQ(true, request.get_priority());
}

TEST(MessageTest, InvokeRequest__target_path) {
  InvokeRequestMessage request;

  EXPECT_EQ("", request.get_target_path());
  request.set_target_path("path/to/node");
  EXPECT_EQ("path/to/node", request.get_target_path());
}

TEST(MessageTest, InvokeRequest__permission_token) {
  // TODO: to be implemented
  InvokeRequestMessage request;

  EXPECT_EQ("", request.get_permission_token());
  request.set_permission_token("permission-token");
  EXPECT_EQ("permission-token", request.get_permission_token());
}

TEST(MessageTest, InvokeRequest__no_stream) {
  InvokeRequestMessage request;

  EXPECT_EQ(false, request.get_no_stream());
  request.set_no_stream(true);
  EXPECT_EQ(true, request.get_no_stream());
}

TEST(MessageTest, InvokeRequest__write) {
  InvokeRequestMessage request;

  request.set_target_path("path/to/dsa");
  request.set_no_stream(true);

  request.size();

  uint8_t buf[1024];
  request.write(buf);

  uint8_t expected_values[] = {0x1e, 0x0,  0x0,  0x0,  0x1e, 0x0,  0x3,  0x0,
                               0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x80,
                               0x0b, 0x0,  0x70, 0x61, 0x74, 0x68, 0x2f, 0x74,
                               0x6f, 0x2f, 0x64, 0x73, 0x61, 0x11};

  EXPECT_EQ(0, memcmp(expected_values, buf,
                      sizeof(expected_values) / sizeof(uint8_t)));
}

TEST(MessageTest, InvokeResponse__Constructor) {
  InvokeResponseMessage response;

  EXPECT_EQ(15, response.size());
  EXPECT_EQ(0, response.get_sequence_id());
  EXPECT_EQ(0, response.get_page_id());
  EXPECT_EQ(MessageType::InvokeResponse, response.type());
  EXPECT_EQ(false, response.is_request());
  EXPECT_EQ(0, response.request_id());
}

TEST(MessageTest, InvokeResponse__source_path) {
  InvokeResponseMessage response;

  EXPECT_EQ("", response.get_source_path());
  response.set_source_path("/source/path");
  EXPECT_EQ("/source/path", response.get_source_path());
}

TEST(MessageTest, InvokeResponse__status) {
  InvokeResponseMessage response;

  static const MessageStatus message_status_all[]{
      MessageStatus::Ok,
      MessageStatus::Initializing,
      MessageStatus::Refreshed,
      MessageStatus::NotAvailable,
      MessageStatus::Closed,
      MessageStatus::Disconnected,
      MessageStatus::PermissionDenied,
      MessageStatus::InvalidMessage,
      MessageStatus::InvalidParameter,
      MessageStatus::Busy,
      MessageStatus::AliasLoop,
      MessageStatus::ConnectionError,
  };
  for (const auto status : message_status_all) {
    response.set_status(status);
    EXPECT_EQ(status, response.get_status());
  }
}

TEST(MessageTest, InvokeResponse__write) {
  InvokeResponseMessage response;

  response.set_source_path("source/path"); // no effect
  response.set_status(MessageStatus::Busy);

  response.size();

  uint8_t buf[1024];
  response.write(buf);

  uint8_t expected_values[] = {0x11, 0x0, 0x0, 0x0, 0x11, 0x0, 0x83, 0x0, 0x0,
                               0x0,  0x0, 0x0, 0x0, 0x0,  0x0, 0x0,  0x28};

  EXPECT_EQ(0, memcmp(expected_values, buf,
                      sizeof(expected_values) / sizeof(uint8_t)));
}
