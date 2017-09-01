#include "dsa/message.h"
#include "gtest/gtest.h"

using namespace dsa;

class ListRequestMessageExt : public ListRequestMessage {
 public:
  ListRequestMessageExt() : ListRequestMessage() {}

  bool check_static_headers(uint8_t *expected_values, size_t size) {
    uint8_t buf[1024];
    static_headers.write(buf);

    return (memcmp(expected_values, buf, size) == 0);
  }
};

class ListResponseMessageExt : public ListResponseMessage {
 public:
  ListResponseMessageExt() : ListResponseMessage() {}

  bool check_static_headers(uint8_t *expected_values, size_t size) {
    uint8_t buf[1024];
    static_headers.write(buf);

    return (memcmp(expected_values, buf, size) == 0);
  }
};

TEST(MessageTest, ListRequest__Constructor_01) {
  // public methods
  // ListRequestMessage();

  ListRequestMessage request;

  EXPECT_EQ(15, request.size());
  EXPECT_EQ(0, request.get_sequence_id());
  EXPECT_EQ(0, request.get_page_id());
  EXPECT_EQ(MessageType::LIST_REQUEST, request.type());
  EXPECT_TRUE(request.is_request());
  EXPECT_EQ(0, request.get_rid());

  EXPECT_FALSE(request.get_priority());
  EXPECT_EQ("", request.get_target_path());
  EXPECT_EQ("", request.get_permission_token());
  EXPECT_FALSE(request.get_no_stream());
  EXPECT_EQ(0, request.get_alias_count());
}

TEST(MessageTest, ListRequest__Constructor_02) {
  //   ListRequestMessage(const ListRequestMessage&);

  const ListRequestMessage src__request;
  ListRequestMessage target__request(src__request);

  EXPECT_EQ(15, target__request.size());
  EXPECT_EQ(0, target__request.get_sequence_id());
  EXPECT_EQ(0, target__request.get_page_id());
  EXPECT_EQ(MessageType::LIST_REQUEST, target__request.type());
  EXPECT_TRUE(target__request.is_request());
  EXPECT_EQ(0, target__request.get_rid());

  EXPECT_FALSE(target__request.get_priority());
  EXPECT_EQ("", target__request.get_target_path());
  EXPECT_EQ("", target__request.get_permission_token());
  EXPECT_FALSE(target__request.get_no_stream());
  EXPECT_EQ(0, target__request.get_alias_count());

  std::string target_path("path/to/abc");
  target__request.set_target_path(target_path);

  EXPECT_EQ("", src__request.get_target_path());
  EXPECT_EQ(target_path, target__request.get_target_path());

  EXPECT_EQ(29, target__request.size());
}

TEST(MessageTest, ListRequest__Constructor_03) {
  //   ListRequestMessage(const uint8_t* data, size_t size);

  const uint8_t data[] = {0xf, 0x0, 0x0, 0x0, 0xf, 0x0, 0x2, 0x0,
                          0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
  size_t data_size = sizeof(data) / sizeof(uint8_t);

  ListRequestMessage request(data, data_size);

  EXPECT_EQ(15, request.size());
  EXPECT_EQ(0, request.get_sequence_id());
  EXPECT_EQ(0, request.get_page_id());
  EXPECT_EQ(MessageType::LIST_REQUEST, request.type());
  EXPECT_TRUE(request.is_request());
  EXPECT_EQ(0, request.get_rid());

  EXPECT_FALSE(request.get_priority());
  EXPECT_EQ("", request.get_target_path());
  EXPECT_EQ("", request.get_permission_token());
  EXPECT_FALSE(request.get_no_stream());
  EXPECT_EQ(0, request.get_alias_count());
}

TEST(MessageTest, ListRequest__Constructor_04) {
  //   ListRequestMessage(const uint8_t* data, size_t size);

  ListRequestMessage request;
  request.set_target_path("/request");
  ListRequestMessage other = request;
  EXPECT_EQ("/request", other.get_target_path());
  other.set_target_path("/other");
  EXPECT_EQ("/request", request.get_target_path());
  EXPECT_EQ("/other", other.get_target_path());

  EXPECT_EQ(24, other.size());
  EXPECT_EQ(0, other.get_sequence_id());
  EXPECT_EQ(0, other.get_page_id());
  EXPECT_EQ(MessageType::LIST_REQUEST, other.type());
  EXPECT_TRUE(other.is_request());
  EXPECT_EQ(0, other.get_rid());

  EXPECT_FALSE(other.get_priority());
  EXPECT_EQ("/other", other.get_target_path());
  EXPECT_EQ("", other.get_permission_token());
  EXPECT_FALSE(other.get_no_stream());
  EXPECT_EQ(0, other.get_alias_count());
}

TEST(MessageTest, ListRequest__get_list_options) {
  //   SetOptions get_list_options() const;

  ListRequestMessage request;
  ListOptions options = request.get_list_options();

  EXPECT_EQ(1, sizeof(options));
}

TEST(MessageTest, ListRequest__update_static_header) {
  // void update_static_header();
  ListRequestMessageExt request;
  request.size();

  uint8_t expect_values[] = {0xf, 0x0, 0x0, 0x0, 0xf, 0x0};
  EXPECT_TRUE(request.check_static_headers(
      expect_values, sizeof(expect_values) / sizeof(uint8_t)));
}

TEST(MessageTest, ListRequest__priority) {
  ListRequestMessage request;

  EXPECT_FALSE(request.get_priority());
  request.set_priority(true);
  EXPECT_TRUE(request.get_priority());
}

TEST(MessageTest, ListRequest__target_path) {
  ListRequestMessage request;

  EXPECT_EQ("", request.get_target_path());
  request.set_target_path("path/to/node");
  EXPECT_EQ("path/to/node", request.get_target_path());
}

TEST(MessageTest, ListRequest__permission_token) {
  // TODO: to be implemented
  ListRequestMessage request;

  EXPECT_EQ("", request.get_permission_token());
  request.set_permission_token("permission-token");
  EXPECT_EQ("permission-token", request.get_permission_token());
}

TEST(MessageTest, ListRequest__no_stream) {
  ListRequestMessage request;

  EXPECT_FALSE(request.get_no_stream());
  request.set_no_stream(true);
  EXPECT_TRUE(request.get_no_stream());
}

TEST(MessageTest, ListRequest__write) {
  ListRequestMessage request;

  request.set_target_path("path/to/dsa");
  request.set_no_stream(true);

  request.size();

  uint8_t buf[1024];
  request.write(buf);

  uint8_t expected_values[] = {0x1e, 0x0,  0x0,  0x0,  0x1e, 0x0,  0x2,  0x0,
                               0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x80,
                               0x0b, 0x0,  0x70, 0x61, 0x74, 0x68, 0x2f, 0x74,
                               0x6f, 0x2f, 0x64, 0x73, 0x61, 0x11};

  EXPECT_EQ(0, memcmp(expected_values, buf,
                      sizeof(expected_values) / sizeof(uint8_t)));
}

TEST(MessageTest, ListRequest__dynamic_structure) {
  ListRequestMessage request;

  //    request.set_status(MessageStatus::CLOSED);
  request.set_sequence_id(1234);  // no effect
  request.set_page_id(4321);      // no effect
  request.set_alias_count(11);
  request.set_priority(true);
  request.set_no_stream(true);
  request.set_permission_token("ptoken");
  request.set_target_path("/target/path");

  request.size();

  uint8_t buf[1024];
  request.write(buf);

  uint8_t expected_values[] = {
      0x2b, 0x0,  0x0,  0x0,  0x2b, 0x0,  0x02, 0x0,  0x0,  0x0,  0x0,
      0x0,  0x0,  0x0,  0x0,  0x10, 0x08, 0x0b, 0x80, 0x0c, 0x0,  0x2f,
      0x74, 0x61, 0x72, 0x67, 0x65, 0x74, 0x2f, 0x70, 0x61, 0x74, 0x68,
      0x60, 0x06, 0x00, 0x70, 0x74, 0x6f, 0x6b, 0x65, 0x6e, 0x11};

  EXPECT_EQ(0, memcmp(expected_values, buf,
                      sizeof(expected_values) / sizeof(uint8_t)));
}

TEST(MessageTest, ListResponse__Constructor) {
  ListResponseMessage response;

  EXPECT_EQ(15, response.size());
  EXPECT_EQ(0, response.get_sequence_id());
  EXPECT_EQ(0, response.get_page_id());
  EXPECT_EQ(MessageType::LIST_RESPONSE, response.type());
  EXPECT_FALSE(response.is_request());
  EXPECT_EQ(0, response.get_rid());
}

TEST(MessageTest, ListResponse__source_path) {
  ListResponseMessage response;

  EXPECT_EQ("", response.get_source_path());
  response.set_source_path("/source/path");
  EXPECT_EQ("/source/path", response.get_source_path());
}

TEST(MessageTest, ListResponse__status) {
  ListResponseMessage response;

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

TEST(MessageTest, ListResponse__write) {
  ListResponseMessage response;

  response.set_source_path("source/path");
  response.set_status(MessageStatus::BUSY);

  response.size();

  uint8_t buf[1024];
  response.write(buf);

  uint8_t expected_values[] = {0x1f, 0x0,  0x0,  0x0,  0x1f, 0x0,  0x82, 0x0,
                               0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
                               0x28, 0x81, 0x0b, 0x00, 0x73, 0x6f, 0x75, 0x72,
                               0x63, 0x65, 0x2f, 0x70, 0x61, 0x74, 0x68};

  EXPECT_EQ(0, memcmp(expected_values, buf,
                      sizeof(expected_values) / sizeof(uint8_t)));
}

TEST(MessageTest, ListResponse__dynamic_structure) {
  ListResponseMessage response;

  response.set_status(MessageStatus::CLOSED);
  response.set_sequence_id(1234);
  response.set_page_id(4321);  // no effect
  response.set_base_path("/base/path");
  response.set_source_path("/source/path");

  response.size();

  uint8_t buf[1024];
  response.write(buf);

  uint8_t expected_values[] = {
      0x32, 0x00, 0x00, 0x00, 0x32, 0x00, 0x82, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x01, 0xd2, 0x04,
      0x00, 0x00, 0x21, 0x0a, 0x00, 0x2f, 0x62, 0x61, 0x73, 0x65,
      0x2f, 0x70, 0x61, 0x74, 0x68, 0x81, 0x0c, 0x00, 0x2f, 0x73,
      0x6f, 0x75, 0x72, 0x63, 0x65, 0x2f, 0x70, 0x61, 0x74, 0x68};

  EXPECT_EQ(0, memcmp(expected_values, buf,
                      sizeof(expected_values) / sizeof(uint8_t)));
}
