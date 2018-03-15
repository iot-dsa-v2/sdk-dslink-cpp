#include <gtest/gtest.h>
#include "dsa/util.h"

using namespace dsa;

const string_ str_utf8 =
    u8"! "
    u8"\"#$%&\'()*+,-./"
    u8"0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
    u8"abcdefghijklmnopqrstuvwxyz{|}~\u00A2\u00F7周";

TEST(StringEncodeTest, EncodeMost) {
  string_ encode_str = url_encode(str_utf8);
  EXPECT_EQ(
      encode_str,
      u8"%21%20%22%23%24%25%26%27%28%29%2A%2B%2C%2D%2E%2F0123456789%3A%3B%"
      u8"3C%3D%3E%3F%40ABCDEFGHIJKLMNOPQRSTUVWXYZ%5B%5C%5D%5E%5F%"
      u8"60abcdefghijklmnopqrstuvwxyz%7B%7C%7D%7E%C2%A2%C3%B7%E5%91%A8");
  EXPECT_EQ(str_utf8, url_decode(encode_str));
}
#ifndef __MINGW32__
TEST(StringEncodeTest, EncodeFileName) {
  string_ encode_str = url_encode_file_name(str_utf8);
  EXPECT_EQ(
      encode_str,
      u8"%21 "
      u8"\"#$%25&%27()%2A+,-.%2F0123456789%3A%3B%3C%3D%3E%3F@"
      u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ[%5C]^_%60abcdefghijklmnopqrstuvwxyz{|}~"
      u8"\u00A2\u00F7周");
  EXPECT_EQ(str_utf8, url_decode(encode_str));
}
#endif
TEST(StringEncodeTest, EncodeNodeName) {
  string_ encode_str = url_encode_node_name(str_utf8);
  EXPECT_EQ(
      encode_str,
      u8"%21 "
      u8"\"#$%25&%27()%2A+,-.%2F0123456789%3A%3B%3C%3D%3E%3F@"
      u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ[%5C]^_%60abcdefghijklmnopqrstuvwxyz{|}~"
      u8"\u00A2\u00F7周");
  EXPECT_EQ(str_utf8, url_decode(encode_str));
}