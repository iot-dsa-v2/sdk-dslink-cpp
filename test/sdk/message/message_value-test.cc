#include "dsa/message.h"
#include "dsa/util.h"
#include "gtest/gtest.h"

#include <msgpack.h>
#include <vector>

using namespace dsa;

TEST(MessageValueTest, Constructor01) {
  //    MessageValue(const uint8_t* data, size_t size);

  std::vector<uint8_t> buf;
  size_t buf_size;

  {
    msgpack_sbuffer sbuf;
    msgpack_sbuffer_init(&sbuf);

    msgpack_packer pk;

    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    msgpack_pack_str(&pk, 9);
    msgpack_pack_str_body(&pk, "timestamp", 9);

    buf_size = sbuf.size + sizeof(uint16_t);
    buf.resize(buf_size);

    buf.insert(buf.begin() + sizeof(uint16_t), &sbuf.data[0],
               &sbuf.data[sbuf.size]);

    uint16_t sbuf_size = (sbuf.size & 0xffff);
    memcpy(buf.data(), &sbuf_size, sizeof(uint16_t));

    msgpack_sbuffer_destroy(&sbuf);
  }

  {
    msgpack_sbuffer sbuf;
    msgpack_sbuffer_init(&sbuf);

    msgpack_packer pk;

    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

    msgpack_pack_str(&pk, 5);
    msgpack_pack_str_body(&pk, "hello", 5);

    buf_size += sbuf.size;
    buf.resize(buf_size);

    buf.insert(buf.begin() + buf_size - sbuf.size, &sbuf.data[0],
               &sbuf.data[sbuf.size]);
    msgpack_sbuffer_destroy(&sbuf);
  }
  // not a valid SUBSCRIBE_RESPONSE message, but is enough for this test
  ResponseMessage msg(MessageType::SUBSCRIBE_RESPONSE);
  msg.set_body(new RefCountBytes(std::move(buf)));

  MessageValue mv(&msg);

  EXPECT_TRUE(mv.meta.is_string());
  EXPECT_EQ("timestamp", mv.meta.get_string());
  EXPECT_TRUE(mv.value.is_string());
  EXPECT_EQ("hello", mv.value.get_string());
}

TEST(MessageValueTest, Constructor02) {
  //  MessageValue(Var value);
  {
    Var v(123);

    MessageValue mv = MessageValue(v);

    EXPECT_TRUE(mv.value.is_int());
    EXPECT_EQ(123, mv.value.get_int());

    EXPECT_TRUE(v.is_int());
    EXPECT_EQ(123, v.get_int());
  }

  {
    Var v("hello");

    MessageValue mv = MessageValue(v);

    EXPECT_TRUE(mv.value.is_string());
    EXPECT_EQ("hello", mv.value.get_string());

    EXPECT_TRUE(v.is_string());
    EXPECT_EQ("hello", v.get_string());
  }

  {
    //  Var v{{"dsid", "dsid-1234"}};

    Var v = {{"dsid", Var("dsid-1234")}};

    MessageValue mv(v);

    EXPECT_TRUE(mv.value.is_map());
    VarMap vm = mv.value.get_map();
    EXPECT_TRUE(vm["dsid"].is_string());
    EXPECT_EQ("dsid-1234", vm["dsid"].get_string());

    EXPECT_TRUE(v.is_map());
  }
}

TEST(MessageValueTest, Constructor03) {
  const char* timestamp = "2000-01-01T00:00:00.000+00:00";

  //  MessageValue(Var value, const string_& ts);
  {
    Var v("hello");

    MessageValue mv = MessageValue(Var(v), timestamp);

    EXPECT_TRUE(mv.value.is_string());
    EXPECT_EQ("hello", mv.value.get_string());

    EXPECT_TRUE(mv.meta.is_map());
    VarMap& map = mv.meta.get_map();
    EXPECT_EQ(1, map.size());
    EXPECT_TRUE(map["ts"].is_string());
    EXPECT_EQ(timestamp, map["ts"].get_string());

    EXPECT_TRUE(v.is_string());
    EXPECT_EQ("hello", v.get_string());
  }

  {
    Var v = {{"dsid", Var("dsid-1234")}};

    MessageValue mv(Var(v), timestamp);

    EXPECT_TRUE(mv.value.is_map());
    VarMap vm = mv.value.get_map();
    EXPECT_TRUE(vm["dsid"].is_string());
    EXPECT_EQ("dsid-1234", vm["dsid"].get_string());

    EXPECT_TRUE(mv.meta.is_map());
    VarMap& map = mv.meta.get_map();
    EXPECT_EQ(1, map.size());
    EXPECT_TRUE(map["ts"].is_string());
    EXPECT_EQ(timestamp, map["ts"].get_string());

    EXPECT_TRUE(v.is_map());
  }
}
