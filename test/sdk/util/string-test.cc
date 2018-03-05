#include <gtest/gtest.h>
#include "dsa/util.h"

using namespace dsa;

const string_ str_utf8 =
    "! "
    "\"#$%&\'()*+,-./"
    "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
    "abcdefghijklmnopqrstuvwxyz{|}~\xA2\xF7";

TEST(StringEncodeTest, EncodeMost) {
  string_ encode_str = url_encode(str_utf8);
  EXPECT_EQ(encode_str,
            "%21%20%22%23%24%25%26%27%28%29%2A%2B%2C%2D%2E%2F0123456789%3A%3B%"
                "3C%3D%3E%3F%40ABCDEFGHIJKLMNOPQRSTUVWXYZ%5B%5C%5D%5E%5F%"
                "60abcdefghijklmnopqrstuvwxyz%7B%7C%7D%7E%A2%F7");
  EXPECT_EQ(str_utf8, url_decode(encode_str));
}
TEST(StringEncodeTest, EncodeFileName) {
  string_ encode_str = url_encode_file_name(str_utf8);
  EXPECT_EQ(encode_str,
            "%21 "
                "\"#$%25&%27()%2A+,-.%2F0123456789%3A%3B%3C%3D%3E%3F@"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ[%5C]^_%60abcdefghijklmnopqrstuvwxyz{|}~"
                "\xA2\xF7");
  EXPECT_EQ(str_utf8, url_decode(encode_str));
}
TEST(StringEncodeTest, EncodeNodeName) {
  string_ encode_str = url_encode_node_name(str_utf8);
  EXPECT_EQ(encode_str,
            "%21 "
                "\"#$%25&%27()%2A+,-.%2F0123456789%3A%3B%3C%3D%3E%3F@"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ[%5C]^_%60abcdefghijklmnopqrstuvwxyz{|}~"
                "\xA2\xF7");
  EXPECT_EQ(str_utf8, url_decode(encode_str));
}

//wide string tests
// no need to use wstring in linux

#ifdef __unix__
const std::string str_wide =
    "! "
        "\"#$%&\'()*+,-./"
        "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
        "abcdefghijklmnopqrstuvwxyz{|}~\u00a2\u00f7周";
TEST(StringEncodeTest, EncodeMostWide) {
  string_ encode_str = url_encode(str_wide);
  EXPECT_EQ(encode_str,
            "%21%20%22%23%24%25%26%27%28%29%2A%2B%2C%2D%2E%2F0123456789%3A%3B%"
                "3C%3D%3E%3F%40ABCDEFGHIJKLMNOPQRSTUVWXYZ%5B%5C%5D%5E%5F%"
                "60abcdefghijklmnopqrstuvwxyz%7B%7C%7D%7E%C2%A2%C3%B7%E5%91%A8");
  EXPECT_EQ(str_wide, url_decode(encode_str));
}
TEST(StringEncodeTest, EncodeFileNameWide) {
  std::string encode_str = url_encode_file_name(str_wide);
  EXPECT_TRUE(
      encode_str ==
          "%21 "
              "\"#$%25&%27()%2A+,-.%2F0123456789%3A%3B%3C%3D%3E%3F@"
              "ABCDEFGHIJKLMNOPQRSTUVWXYZ[%5C]^_%60abcdefghijklmnopqrstuvwxyz{|}~¢÷周");
  EXPECT_TRUE(str_wide == url_decode(encode_str));
}
TEST(StringEncodeTest, EncodeNodeNameWide) {
  std::string encode_str = url_encode_node_name(str_wide);
  EXPECT_TRUE(
      encode_str ==
          "%21 "
              "\"#$%25&%27()%2A+,-.%2F0123456789%3A%3B%3C%3D%3E%3F@"
              "ABCDEFGHIJKLMNOPQRSTUVWXYZ[%5C]^_%60abcdefghijklmnopqrstuvwxyz{|}~¢÷周");
  EXPECT_TRUE(str_wide == url_decode(encode_str));
}

#elif defined(_WIN32) || defined(_WIN64)
const std::wstring str_wide =
    L"! "
    L"\"#$%&\'()*+,-./"
    L"0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
    L"abcdefghijklmnopqrstuvwxyz{|}~\u00a2\u00f7周";
TEST(StringEncodeTest, EncodeMostWide) {
  string_ encode_str = url_encode(str_wide);
  EXPECT_EQ(encode_str,
            "%21%20%22%23%24%25%26%27%28%29%2A%2B%2C%2D%2E%2F0123456789%3A%3B%"
            "3C%3D%3E%3F%40ABCDEFGHIJKLMNOPQRSTUVWXYZ%5B%5C%5D%5E%5F%"
            "60abcdefghijklmnopqrstuvwxyz%7B%7C%7D%7E%C2%A2%C3%B7%E5%91%A8");
  EXPECT_EQ(str_wide, url_decode_w(encode_str));
}
TEST(StringEncodeTest, EncodeFileNameWide) {
  std::wstring encode_str = url_encode_file_name_w(str_wide);
  EXPECT_TRUE(
      encode_str ==
      L"%21 "
      L"\"#$%25&%27()%2A+,-.%2F0123456789%3A%3B%3C%3D%3E%3F@"
      L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[%5C]^_%60abcdefghijklmnopqrstuvwxyz{|}~¢÷周");
  EXPECT_TRUE(str_wide == url_decode_w(encode_str));
}
TEST(StringEncodeTest, EncodeNodeNameWide) {
  std::wstring encode_str = url_encode_node_name_w(str_wide);
  EXPECT_TRUE(
      encode_str ==
      L"%21 "
      L"\"#$%25&%27()%2A+,-.%2F0123456789%3A%3B%3C%3D%3E%3F@"
      L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[%5C]^_%60abcdefghijklmnopqrstuvwxyz{|}~¢÷周");
  EXPECT_TRUE(str_wide == url_decode_w(encode_str));
}
#endif