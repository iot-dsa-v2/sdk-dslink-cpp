#include <dsa/util.h>
#include "dsa/message.h"
#include "dsa/util.h"
#include "gtest/gtest.h"

#include <msgpack.h>

using namespace dsa;

TEST(MessageValueTest, Constructor_01) {
  //   TODO : implement this
  //    MessageValue(const uint8_t* data, size_t size);
}

TEST(MessageValueTest, Constructor_02) {
  //  MessageValue(Variant value);
{
  Variant v(123);

  MessageValue mv = MessageValue(v);

  EXPECT_TRUE(mv.value.is_int());
  EXPECT_EQ(123, mv.value.get_int());

  EXPECT_TRUE(v.is_int());
  EXPECT_EQ(123, v.get_int());
}

{
  Variant v("hello");

  MessageValue mv = MessageValue(v);

  EXPECT_TRUE(mv.value.is_string());
  EXPECT_EQ("hello", mv.value.get_string());

  EXPECT_TRUE(v.is_string());
  EXPECT_EQ("hello", v.get_string());
}

{
//  Variant v{{"dsid", "dsid-1234"}};

  Variant v = {{"dsid", Variant("dsid-1234")}};

  MessageValue mv(v);

  EXPECT_TRUE(mv.value.is_map());
  VariantMap vm = mv.value.get_map();
  EXPECT_TRUE(vm["dsid"].is_string());
  EXPECT_EQ("dsid-1234", vm["dsid"].get_string());

  EXPECT_TRUE(v.is_map());

}
}


TEST(MessageValueTest, Constructor_03) {
//  MessageValue(Variant value, const std::string& ts);

{
  Variant v("hello");

  MessageValue mv = MessageValue(v, "timestamp");

  EXPECT_TRUE(mv.value.is_string());
  EXPECT_EQ("hello", mv.value.get_string());

  EXPECT_TRUE(mv.meta.is_map());
  VariantMap& map = mv.meta.get_map();
  EXPECT_EQ(1, map.size());
  EXPECT_TRUE(map["ts"].is_string());
  EXPECT_EQ("timestamp", map["ts"].get_string());

  EXPECT_TRUE(v.is_string());
  EXPECT_EQ("hello", v.get_string());
}

{
  Variant v = {{"dsid", Variant("dsid-1234")}};

  MessageValue mv(v, "timestamp");

  EXPECT_TRUE(mv.value.is_map());
  VariantMap vm = mv.value.get_map();
  EXPECT_TRUE(vm["dsid"].is_string());
  EXPECT_EQ("dsid-1234", vm["dsid"].get_string());

  EXPECT_TRUE(mv.meta.is_map());
  VariantMap& map = mv.meta.get_map();
  EXPECT_EQ(1, map.size());
  EXPECT_TRUE(map["ts"].is_string());
  EXPECT_EQ("timestamp", map["ts"].get_string());

  EXPECT_TRUE(v.is_map());
}
}





