#include "dsa/message.h"
#include "gtest/gtest.h"

using namespace dsa;

TEST(MessageTest, SetRequest__MessageConstruction_01) {
  // public methods
  // SetRequestMessage();

  SetRequestMessage request;

  EXPECT_EQ(15, request.size());
  EXPECT_EQ(0, request.get_sequence_id());
  EXPECT_EQ(0, request.get_page_id());
  EXPECT_EQ(MessageType::SetRequest, request.type());
  EXPECT_EQ(true, request.is_request());
  EXPECT_EQ(0, request.request_id());

  EXPECT_EQ(0, request.get_priority());
  EXPECT_EQ("", request.get_target_path());
  EXPECT_EQ("", request.get_permission_token());
  EXPECT_EQ(false, request.get_no_stream());
  EXPECT_EQ(0, request.get_alias_count());
}

TEST(MessageTest, SetRequest__MessageConstruction_02) {
  //   SetRequestMessage(const SetRequestMessage&);

  const SetRequestMessage src__request;
  SetRequestMessage target__request(src__request);

  EXPECT_EQ(15, target__request.size());
  EXPECT_EQ(0, target__request.get_sequence_id());
  EXPECT_EQ(0, target__request.get_page_id());
  EXPECT_EQ(MessageType::SetRequest, target__request.type());
  EXPECT_EQ(true, target__request.is_request());
  EXPECT_EQ(0, target__request.request_id());

  EXPECT_EQ(0, target__request.get_priority());
  EXPECT_EQ("", target__request.get_target_path());
  EXPECT_EQ("", target__request.get_permission_token());
  EXPECT_EQ(false, target__request.get_no_stream());
  EXPECT_EQ(0, target__request.get_alias_count());

  std::string target_path("path/to/abc");
  target__request.set_target_path(target_path);

  EXPECT_EQ("", src__request.get_target_path());
  EXPECT_EQ(target_path, target__request.get_target_path());
}

TEST(MessageTest, SetRequest__MessageConstruction_03) {
  //   SetRequestMessage(const uint8_t* data, size_t size);

  const uint8_t data[] = {0xf, 0x0, 0x0, 0x0,
                          0xf, 0x0, 0x4, 0x0,
                          0x0, 0x0, 0x0, 0x0,
                          0x0, 0x0, 0x0};
  size_t data_size = sizeof(data)/sizeof(uint8_t);

  SetRequestMessage request(data, data_size);

  EXPECT_EQ(15, request.size());
  EXPECT_EQ(0, request.get_sequence_id());
  EXPECT_EQ(0, request.get_page_id());
  EXPECT_EQ(MessageType::SetRequest, request.type());
  EXPECT_EQ(true, request.is_request());
  EXPECT_EQ(0, request.request_id());

  EXPECT_EQ(0, request.get_priority());
  EXPECT_EQ("", request.get_target_path());
  EXPECT_EQ("", request.get_permission_token());
  EXPECT_EQ(false, request.get_no_stream());
  EXPECT_EQ(0, request.get_alias_count());
}

TEST(MessageTest, SetRequest__MessageConstruction_04) {
  //   SetRequestMessage(const uint8_t* data, size_t size);

  SetRequestMessage request;
  request.set_target_path("/request");
  SetRequestMessage other = request;
  EXPECT_EQ("/request", other.get_target_path());
  other.set_target_path("/other");
  EXPECT_EQ("/request", request.get_target_path());
  EXPECT_EQ("/other", other.get_target_path());

  EXPECT_EQ(24, other.size());
  EXPECT_EQ(0, other.get_sequence_id());
  EXPECT_EQ(0, other.get_page_id());
  EXPECT_EQ(MessageType::SetRequest, other.type());
  EXPECT_EQ(true, other.is_request());
  EXPECT_EQ(0, other.request_id());

  EXPECT_EQ(0, other.get_priority());
  EXPECT_EQ("/other", other.get_target_path());
  EXPECT_EQ("", other.get_permission_token());
  EXPECT_EQ(false, other.get_no_stream());
  EXPECT_EQ(0, other.get_alias_count());
}

TEST(MessageTest, SetRequest__get_set_options) {
  //   SetOptions get_set_options() const;

  SetRequestMessage request;
  SetOptions option = request.get_set_options();

  EXPECT_EQ(1, sizeof(option));
}

TEST(MessageTest, SetRequest__update_static_header) {
  // void update_static_header();

  class SetRequestMessageExt : public SetRequestMessage{
  public:
    SetRequestMessageExt() :
      SetRequestMessage() {}

    void update_static_header_ext() {
      SetRequestMessage::update_static_header();
    }

    bool check_static_headers(uint8_t *expected_values, size_t size) {
      uint8_t buf[1024];
      static_headers.write(buf);

      return (memcmp(expected_values, buf, size) == 0);
    }
  };

  SetRequestMessageExt request;
  request.update_static_header_ext();

  uint8_t expect_values[] = {0xf, 0x0, 0x0, 0x0, 0xf, 0x0};
  EXPECT_EQ(true, request.check_static_headers(expect_values, 
					    sizeof(expect_values)/sizeof(uint8_t)));
}
