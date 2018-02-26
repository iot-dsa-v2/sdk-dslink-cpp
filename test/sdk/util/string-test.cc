#include <gtest/gtest.h>
#include "dsa/util.h"

using namespace dsa;

const string_ str =
    "! "
    "\"#$%&\'()*+,-./"
    "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
    "abcdefghijklmnopqrstuvwxyz{|}~\u00a2\u00f7";
TEST(StringEncodeTest, EncodeMost) {
  string_ encode_str = url_encode(str);
  EXPECT_EQ(encode_str,
            "%21%20%22%23%24%25%26%27%28%29%2A%2B%2C%2D%2E%2F0123456789%3A%3B%"
            "3C%3D%3E%3F%40ABCDEFGHIJKLMNOPQRSTUVWXYZ%5B%5C%5D%5E%5F%"
            "60abcdefghijklmnopqrstuvwxyz%7B%7C%7D%7E%C2%A2%C3%B7");
  EXPECT_EQ(str, url_decode(encode_str));
}
TEST(StringEncodeTest, EncodeFileName) {
  string_ encode_str = url_encode_file_name(str);
  EXPECT_EQ(encode_str,
            "%21 "
            "\"#$%25&%27()%2A+,-.%2F0123456789%3A%3B%3C%3D%3E%3F@"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ[%5C]^_%60abcdefghijklmnopqrstuvwxyz{|}~"
            "%C2%A2%C3%B7");
  EXPECT_EQ(str, url_decode(encode_str));
}
TEST(StringEncodeTest, EncodeNodeName) {
  string_ encode_str = url_encode_node_name(str);
  EXPECT_EQ(encode_str,
            "%21 "
            "\"#$%25&%27()%2A+,-.%2F0123456789%3A%3B%3C%3D%3E%3F@"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ[%5C]^_%60abcdefghijklmnopqrstuvwxyz{|}~"
            "¢÷");
  EXPECT_EQ(str, url_decode(encode_str));
}
