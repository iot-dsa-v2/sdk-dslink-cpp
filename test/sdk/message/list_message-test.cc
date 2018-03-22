#include "dsa/message.h"
#include <gtest/gtest.h>

using namespace dsa;

bool check_static_headers(ListRequestMessage& message, uint8_t* expected_values,
                          size_t size) {
  uint8_t buf[1024];
  message.write(buf);

  return (memcmp(expected_values, buf, size) == 0);
}

TEST(MessageTest, ListRequestConstructor01) {
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
  EXPECT_EQ("", request.get_target_path().full_str());
  EXPECT_EQ("", request.get_permission_token());
  EXPECT_FALSE(request.get_no_stream());
  EXPECT_EQ(0, request.get_alias_count());
}

TEST(MessageTest, ListRequestConstructor02) {
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

TEST(MessageTest, ListRequestConstructor03) {
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
  EXPECT_EQ("", request.get_target_path().full_str());
  EXPECT_EQ("", request.get_permission_token());
  EXPECT_FALSE(request.get_no_stream());
  EXPECT_EQ(0, request.get_alias_count());
}

TEST(MessageTest, ListRequestConstructor04) {
  //   ListRequestMessage(const uint8_t* data, size_t size);

  ListRequestMessage request;
  request.set_target_path("/request");
  ListRequestMessage other = request;
  EXPECT_EQ("/request", other.get_target_path().full_str());
  other.set_target_path("/other");
  EXPECT_EQ("/request", request.get_target_path().full_str());
  EXPECT_EQ("/other", other.get_target_path().full_str());

  EXPECT_EQ(24, other.size());
  EXPECT_EQ(0, other.get_sequence_id());
  EXPECT_EQ(0, other.get_page_id());
  EXPECT_EQ(MessageType::LIST_REQUEST, other.type());
  EXPECT_TRUE(other.is_request());
  EXPECT_EQ(0, other.get_rid());

  EXPECT_FALSE(other.get_priority());
  EXPECT_EQ("/other", other.get_target_path().full_str());
  EXPECT_EQ("", other.get_permission_token());
  EXPECT_FALSE(other.get_no_stream());
  EXPECT_EQ(0, other.get_alias_count());
}

TEST(MessageTest, ListRequestConstructor05) {
  ListRequestMessage source_request;

  source_request.set_sequence_id(1234);  // no effect
  source_request.set_page_id(4321);      // no effect
  source_request.set_alias_count(11);
  source_request.set_no_stream(true);
  source_request.set_permission_token("ptoken");
  source_request.set_target_path("/target/path");

  source_request.size();

  ListRequestMessage dup_request(source_request);

  uint8_t src_buf[1024];
  dup_request.write(src_buf);

  //
  size_t buf_size = 42;
  ListRequestMessage request(src_buf, buf_size);

  request.size();

  uint8_t buf[1024];
  request.write(buf);

  EXPECT_EQ(0, memcmp(src_buf, buf, buf_size));
}

TEST(MessageTest, ListRequestGetListOptions) {
  //   SetOptions get_list_options() const;

  ListRequestMessage request;
  ListOptions options = request.get_list_options();

  EXPECT_EQ(1, sizeof(options));
}

TEST(MessageTest, ListRequestUpdateStaticHeader) {
  // void update_static_header();
  ListRequestMessage request;
  request.size();

  uint8_t expect_values[] = {0xf, 0x0, 0x0, 0x0, 0xf, 0x0};
  EXPECT_TRUE(check_static_headers(request, expect_values,
                                   sizeof(expect_values) / sizeof(uint8_t)));
}

TEST(MessageTest, ListRequestPriority) {
  ListRequestMessage request;

  EXPECT_FALSE(request.get_priority());
  request.set_priority(true);
  EXPECT_TRUE(request.get_priority());
}

TEST(MessageTest, ListRequestTargetPath) {
  ListRequestMessage request;

  EXPECT_EQ("", request.get_target_path().full_str());
  request.set_target_path("path/to/node");
  EXPECT_EQ("path/to/node", request.get_target_path().full_str());
}

TEST(MessageTest, ListRequestPermissionToken) {
  // TODO: to be implemented
  ListRequestMessage request;

  EXPECT_EQ("", request.get_permission_token());
  request.set_permission_token("permission-token");
  EXPECT_EQ("permission-token", request.get_permission_token());
}

TEST(MessageTest, ListRequestNoStream) {
  ListRequestMessage request;

  EXPECT_FALSE(request.get_no_stream());
  request.set_no_stream(true);
  EXPECT_TRUE(request.get_no_stream());
}

TEST(MessageTest, ListRequestWrite) {
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

TEST(MessageTest, ListRequestDynamicStructure) {
  ListRequestMessage request;

  //    request.set_status(Status::DONE);
  request.set_sequence_id(1234);  // no effect
  request.set_page_id(4321);      // no effect
  request.set_alias_count(11);
  request.set_no_stream(true);
  request.set_permission_token("ptoken");
  request.set_target_path("/target/path");

  request.size();

  uint8_t buf[1024];
  request.write(buf);

  uint8_t expected_values[] = {
      0x2a, 0x0,  0x0,  0x0,  0x2a, 0x0,  0x02, 0x0,  0x0,  0x0,  0x0,
      0x0,  0x0,  0x0,  0x0,  0x08, 0x0b, 0x80, 0x0c, 0x0,  0x2f, 0x74,
      0x61, 0x72, 0x67, 0x65, 0x74, 0x2f, 0x70, 0x61, 0x74, 0x68, 0x60,
      0x06, 0x00, 0x70, 0x74, 0x6f, 0x6b, 0x65, 0x6e, 0x11};

  EXPECT_EQ(0, memcmp(expected_values, buf,
                      sizeof(expected_values) / sizeof(uint8_t)));
}

TEST(MessageTest, ListResponseConstructor01) {
  ListResponseMessage response;

  EXPECT_EQ(15, response.size());
  EXPECT_EQ(0, response.get_sequence_id());
  EXPECT_EQ(0, response.get_page_id());
  EXPECT_EQ(MessageType::LIST_RESPONSE, response.type());
  EXPECT_FALSE(response.is_request());
  EXPECT_EQ(0, response.get_rid());
  EXPECT_EQ(MessageType::LIST_RESPONSE,
            response.get_response_type(MessageType::LIST_REQUEST));
}

TEST(MessageTest, ListResponseConstructor02) {
  ListResponseMessage source_response;

  source_response.set_status(Status::DONE);
  source_response.set_sequence_id(1234);
  source_response.set_page_id(4321);  // no effect
  source_response.set_pub_path("/base/path");
  source_response.set_source_path("/source/path");

  source_response.size();

  uint8_t src_buf[1024];
  source_response.write(src_buf);

  //
  size_t buf_size = 50;
  ListResponseMessage response(src_buf, buf_size);

  response.size();

  uint8_t buf[1024];
  response.write(buf);

  EXPECT_EQ(0, memcmp(src_buf, buf, buf_size));
}

TEST(MessageTest, ListResponseSourcePath) {
  ListResponseMessage response;

  EXPECT_EQ("", response.get_source_path());
  response.set_source_path("/source/path");
  EXPECT_EQ("/source/path", response.get_source_path());
}

TEST(MessageTest, ListResponseStatus) {
  ListResponseMessage response;

  static const Status message_status_all[]{
      Status::OK,
      Status::INITIALIZING,
      Status::REFRESHED,
      Status::NOT_AVAILABLE,
      Status::DONE,
      Status::DISCONNECTED,
      Status::PERMISSION_DENIED,
      Status::INVALID_MESSAGE,
      Status::INVALID_PARAMETER,
      Status::BUSY,
      Status::ALIAS_LOOP,
      Status::CONNECTION_ERROR,
  };
  for (const auto status : message_status_all) {
    response.set_status(status);
    EXPECT_EQ(status, response.get_status());
  }
}

TEST(MessageTest, ListResponseWrite) {
  ListResponseMessage response;

  response.set_source_path("source/path");
  response.set_status(Status::BUSY);

  response.size();

  uint8_t buf[1024];
  response.write(buf);

  uint8_t expected_values[] = {0x1f, 0x0,  0x0,  0x0,  0x1f, 0x0,  0x82, 0x0,
                               0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
                               0x48, 0x81, 0x0b, 0x00, 0x73, 0x6f, 0x75, 0x72,
                               0x63, 0x65, 0x2f, 0x70, 0x61, 0x74, 0x68};

  EXPECT_EQ(0, memcmp(expected_values, buf,
                      sizeof(expected_values) / sizeof(uint8_t)));
}

TEST(MessageTest, ListResponseDynamicStructure) {
  ListResponseMessage response;

  response.set_status(Status::DONE);
  response.set_sequence_id(1234);
  response.set_page_id(4321);  // no effect
  response.set_pub_path("/base/path");
  response.set_source_path("/source/path");

  response.size();

  uint8_t buf[1024];
  response.write(buf);

  uint8_t expected_values[] = {
      0x32, 0x00, 0x00, 0x00, 0x32, 0x00, 0x82, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x01, 0xd2, 0x04,
      0x00, 0x00, 0x21, 0x0a, 0x00, 0x2f, 0x62, 0x61, 0x73, 0x65,
      0x2f, 0x70, 0x61, 0x74, 0x68, 0x81, 0x0c, 0x00, 0x2f, 0x73,
      0x6f, 0x75, 0x72, 0x63, 0x65, 0x2f, 0x70, 0x61, 0x74, 0x68};

  EXPECT_EQ(0, memcmp(expected_values, buf,
                      sizeof(expected_values) / sizeof(uint8_t)));
}

TEST(MessageTest, ListResponseCopy) {
  ListResponseMessage response;

  response.set_status(Status::DONE);
  response.set_sequence_id(1234);
  response.set_page_id(4321);  // no effect
  response.set_pub_path("/base/path");
  response.set_source_path("/source/path");

  response.size();

  ListResponseMessage dup_response(response);

  uint8_t buf[1024];
  dup_response.write(buf);

  uint8_t expected_values[] = {
      0x32, 0x00, 0x00, 0x00, 0x32, 0x00, 0x82, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x01, 0xd2, 0x04,
      0x00, 0x00, 0x21, 0x0a, 0x00, 0x2f, 0x62, 0x61, 0x73, 0x65,
      0x2f, 0x70, 0x61, 0x74, 0x68, 0x81, 0x0c, 0x00, 0x2f, 0x73,
      0x6f, 0x75, 0x72, 0x63, 0x65, 0x2f, 0x70, 0x61, 0x74, 0x68};

  EXPECT_EQ(0, memcmp(expected_values, buf,
                      sizeof(expected_values) / sizeof(uint8_t)));
}
