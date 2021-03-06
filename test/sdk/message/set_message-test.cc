#include "dsa/message.h"

#include <gtest/gtest.h>

using namespace dsa;

bool check_static_headers(SetRequestMessage& message, uint8_t* expected_values,
                          size_t size) {
  uint8_t buf[1024];
  message.write(buf);

  return (memcmp(expected_values, buf, size) == 0);
}

TEST(MessageTest, SetRequestConstructor01) {
  // public methods
  // SetRequestMessage();

  SetRequestMessage request;

  EXPECT_EQ(15, request.size());
  EXPECT_EQ(0, request.get_sequence_id());
  EXPECT_EQ(0, request.get_page_id());
  EXPECT_EQ(MessageType::SET_REQUEST, request.type());
  EXPECT_TRUE(request.is_request());
  EXPECT_EQ(0, request.get_rid());

  EXPECT_FALSE(request.get_priority());
  EXPECT_EQ("", request.get_target_path().full_str());
  EXPECT_EQ("", request.get_permission_token());
  EXPECT_FALSE(request.get_no_stream());
  EXPECT_EQ(0, request.get_alias_count());
}

TEST(MessageTest, SetRequestConstructor02) {
  //   SetRequestMessage(const SetRequestMessage&);

  const SetRequestMessage src__request;
  SetRequestMessage target__request(src__request);

  EXPECT_EQ(15, target__request.size());
  EXPECT_EQ(0, target__request.get_sequence_id());
  EXPECT_EQ(0, target__request.get_page_id());
  EXPECT_EQ(MessageType::SET_REQUEST, target__request.type());
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

TEST(MessageTest, SetRequestConstructor03) {
  //   SetRequestMessage(const uint8_t* data, size_t size);

  const uint8_t data[] = {0xf, 0x0, 0x0, 0x0, 0xf, 0x0, 0x4, 0x0,
                          0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
  size_t data_size = sizeof(data) / sizeof(uint8_t);

  SetRequestMessage request(data, data_size);

  EXPECT_EQ(15, request.size());
  EXPECT_EQ(0, request.get_sequence_id());
  EXPECT_EQ(0, request.get_page_id());
  EXPECT_EQ(MessageType::SET_REQUEST, request.type());
  EXPECT_TRUE(request.is_request());
  EXPECT_EQ(0, request.get_rid());

  EXPECT_FALSE(request.get_priority());
  EXPECT_EQ("", request.get_target_path().full_str());
  EXPECT_EQ("", request.get_permission_token());
  EXPECT_FALSE(request.get_no_stream());
  EXPECT_EQ(0, request.get_alias_count());
}

TEST(MessageTest, SetRequestConstructor04) {
  //   SetRequestMessage(const uint8_t* data, size_t size);

  SetRequestMessage request;
  request.set_target_path("/request");
  SetRequestMessage other = request;
  EXPECT_EQ("/request", other.get_target_path().full_str());
  other.set_target_path("/other");
  EXPECT_EQ("/request", request.get_target_path().full_str());
  EXPECT_EQ("/other", other.get_target_path().full_str());

  EXPECT_EQ(24, other.size());
  EXPECT_EQ(0, other.get_sequence_id());
  EXPECT_EQ(0, other.get_page_id());
  EXPECT_EQ(MessageType::SET_REQUEST, other.type());
  EXPECT_TRUE(other.is_request());
  EXPECT_EQ(0, other.get_rid());

  EXPECT_FALSE(other.get_priority());
  EXPECT_EQ("/other", other.get_target_path().full_str());
  EXPECT_EQ("", other.get_permission_token());
  EXPECT_FALSE(other.get_no_stream());
  EXPECT_EQ(0, other.get_alias_count());
}

TEST(MessageTest, SetResponseConstructor05) {
  SetRequestMessage source_request;

  source_request.set_sequence_id(1234);  // no effect
  source_request.set_page_id(4321);
  source_request.set_alias_count(11);
  source_request.set_priority(true);
  source_request.set_no_stream(true);
  source_request.set_permission_token("ptoken");
  source_request.set_target_path("/target/path");

  source_request.size();

  SetRequestMessage dup_request(source_request);

  uint8_t src_buf[1024];
  dup_request.write(src_buf);

  //
  size_t buf_size = 48;
  SetRequestMessage request(src_buf, buf_size);

  request.size();

  uint8_t buf[1024];
  request.write(buf);

  EXPECT_EQ(0, memcmp(src_buf, buf, buf_size));
}

TEST(MessageTest, SetRequestUpdateStaticHeader) {
  // void update_static_header();
  SetRequestMessage request;
  request.size();

  uint8_t expect_values[] = {0xf, 0x0, 0x0, 0x0, 0xf, 0x0};
  EXPECT_TRUE(check_static_headers(request, expect_values,
                                   sizeof(expect_values) / sizeof(uint8_t)));
}

TEST(MessageTest, SetRequestPriority) {
  SetRequestMessage request;

  EXPECT_FALSE(request.get_priority());
  request.set_priority(true);
  EXPECT_TRUE(request.get_priority());
}

TEST(MessageTest, SetRequestTargetPath) {
  SetRequestMessage request;

  EXPECT_EQ("", request.get_target_path().full_str());
  request.set_target_path("path/to/node");
  EXPECT_EQ("path/to/node", request.get_target_path().full_str());
}

TEST(MessageTest, SetRequestPermissionToken) {
  // TODO: to be implemented
  SetRequestMessage request;

  EXPECT_EQ("", request.get_permission_token());
  request.set_permission_token("permission-token");
  EXPECT_EQ("permission-token", request.get_permission_token());
}

TEST(MessageTest, SetRequestNoStream) {
  SetRequestMessage request;

  EXPECT_FALSE(request.get_no_stream());
  request.set_no_stream(true);
  EXPECT_TRUE(request.get_no_stream());
}

TEST(MessageTest, SetRequestWrite) {
  SetRequestMessage request;

  request.set_target_path("path/to/dsa");
  request.set_no_stream(true);

  request.size();

  uint8_t buf[1024];
  request.write(buf);

  uint8_t expected_values[] = {0x1e, 0x0,  0x0,  0x0,  0x1e, 0x0,  0x4,  0x0,
                               0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x80,
                               0x0b, 0x0,  0x70, 0x61, 0x74, 0x68, 0x2f, 0x74,
                               0x6f, 0x2f, 0x64, 0x73, 0x61, 0x11};

  EXPECT_EQ(0, memcmp(expected_values, buf,
                      sizeof(expected_values) / sizeof(uint8_t)));
}

TEST(MessageTest, SetRequest__dynamic_structure) {
  SetRequestMessage request;

  //    request.set_status(Status::DONE);
  request.set_sequence_id(1234);  // no effect
  request.set_page_id(4321);
  request.set_alias_count(11);
  request.set_priority(true);
  request.set_no_stream(true);
  //      request.set_qos(1); //
  //      request.set_queue_size();
  //      request.set_queue_time();
  //      request.set_pub_path();
  //      request.skippable();
  //      request.set_max_permission();
  request.set_permission_token("ptoken");
  request.set_target_path("/target/path");
  //    request.set_source_path("/source/path");  // no effect

  request.size();

  uint8_t buf[1024];
  request.write(buf);

  uint8_t expected_values[] = {
      0x30, 0x0,  0x0,  0x0,  0x30, 0x0,  0x04, 0x0,  0x0,  0x0,  0x0,  0x0,
      0x0,  0x0,  0x0,  0x10, 0x02, 0xe1, 0x10, 0x0,  0x0,  0x08, 0x0b, 0x80,
      0x0c, 0x0,  0x2f, 0x74, 0x61, 0x72, 0x67, 0x65, 0x74, 0x2f, 0x70, 0x61,
      0x74, 0x68, 0x60, 0x06, 0x0,  0x70, 0x74, 0x6f, 0x6b, 0x65, 0x6e, 0x11};

  EXPECT_EQ(0, memcmp(expected_values, buf,
                      sizeof(expected_values) / sizeof(uint8_t)));
}

TEST(MessageTest, SetResponse__Constructor_01) {
  SetResponseMessage response;

  EXPECT_EQ(15, response.size());
  EXPECT_EQ(0, response.get_sequence_id());
  EXPECT_EQ(0, response.get_page_id());
  EXPECT_EQ(MessageType::SET_RESPONSE, response.type());
  EXPECT_FALSE(response.is_request());
  EXPECT_EQ(0, response.get_rid());
  EXPECT_EQ(MessageType::SET_RESPONSE,
            response.get_response_type(MessageType::SET_REQUEST));
}

TEST(MessageTest, SetResponse__Constructor_02) {
  SetResponseMessage source_response;
  source_response.set_status(Status::BUSY);

  source_response.size();

  uint8_t src_buf[1024];
  source_response.write(src_buf);

  //
  size_t buf_size = 17;
  SetResponseMessage response(src_buf, buf_size);

  response.size();

  uint8_t buf[1024];
  response.write(buf);

  EXPECT_EQ(0, memcmp(src_buf, buf, buf_size));
}

TEST(MessageTest, SetResponse__source_path) {
  SetResponseMessage response;

  EXPECT_EQ("", response.get_source_path());
  response.set_source_path("/source/path");
  EXPECT_EQ("/source/path", response.get_source_path());
}

TEST(MessageTest, SetResponse__status) {
  SetResponseMessage response;

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

TEST(MessageTest, SetResponse__write) {
  SetResponseMessage response;

  response.set_source_path("source/path");
  response.set_status(Status::BUSY);

  response.size();

  uint8_t buf[1024];
  response.write(buf);

  uint8_t expected_values[] = {0x11, 0x0, 0x0, 0x0, 0x11, 0x0, 0x84, 0x0, 0x0,
                               0x0,  0x0, 0x0, 0x0, 0x0,  0x0, 0x0,  0x48};

  EXPECT_EQ(0, memcmp(expected_values, buf,
                      sizeof(expected_values) / sizeof(uint8_t)));
}

TEST(MessageTest, SetResponse__dynamic_structure) {
  SetResponseMessage response;

  response.set_status(Status::DONE);
  response.set_sequence_id(1234);  // no effect
  response.set_page_id(4321);      // no effect
  //  response.set_alias_count(11);
  //  response.set_priority(true);
  //  response.set_no_stream(true);
  //  response.set_qos(1); //
  //  response.set_queue_size();
  //  response.set_queue_time();
  //  response.set_pub_path();
  //  response.skippable();
  //  response.set_max_permission();
  //  response.set_permission_token();
  //  response.set_target_path("/target/path");
  response.set_source_path("/source/path");  // no effect

  response.size();

  uint8_t buf[1024];
  response.write(buf);

  uint8_t expected_values[] = {0x11, 0x0, 0x0, 0x0, 0x11, 0x0, 0x84, 0x0, 0x0,
                               0x0,  0x0, 0x0, 0x0, 0x0,  0x0, 0x0,  0x20};

  EXPECT_EQ(0, memcmp(expected_values, buf,
                      sizeof(expected_values) / sizeof(uint8_t)));
}

TEST(MessageTest, copy) {
  SetResponseMessage response;

  response.set_source_path("source/path");
  response.set_status(Status::BUSY);

  response.size();

  SetResponseMessage dup_response(response);

  uint8_t buf[1024];
  dup_response.write(buf);

  uint8_t expected_values[] = {0x11, 0x0, 0x0, 0x0, 0x11, 0x0, 0x84, 0x0, 0x0,
                               0x0,  0x0, 0x0, 0x0, 0x0,  0x0, 0x0,  0x48};

  EXPECT_EQ(0, memcmp(expected_values, buf,
                      sizeof(expected_values) / sizeof(uint8_t)));
}
