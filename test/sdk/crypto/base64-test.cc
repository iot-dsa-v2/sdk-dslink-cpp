#include "dsa/crypto.h"
#include "gtest/gtest.h"

#include <cstring>

using namespace dsa;

TEST(Base64Test, Base64_EncodingDecoding) {
  {
    const uint8_t data[] = "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.";
    size_t data_size = sizeof(data)/sizeof(uint8_t);

    std::string encoded = base64_encode(data, data_size);

    EXPECT_EQ("TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=", encoded);

    EXPECT_EQ(0, strncmp((const char*)data, base64_decode(encoded).c_str(), data_size));
  }
  {
    const uint8_t data[] = "sdk-dslink-cpp";
    size_t data_size = sizeof(data)/sizeof(uint8_t);

    std::string encoded = base64_encode(data, data_size);

    EXPECT_EQ("c2RrLWRzbGluay1jcHA=", encoded);

    EXPECT_EQ(0, strncmp((const char*)data, base64_decode(encoded).c_str(), data_size));
  }
  {
    const uint8_t data[] = "-dslink-cpp";
    size_t data_size = sizeof(data)/sizeof(uint8_t);

    std::string encoded = base64_encode(data, data_size);

    EXPECT_EQ("LWRzbGluay1jcHA=", encoded);

    EXPECT_EQ(0, strncmp((const char*)data, base64_decode(encoded).c_str(), data_size));
  }
  {
    const uint8_t data[] = "dslink-cpp";
    size_t data_size = sizeof(data)/sizeof(uint8_t);

    std::string encoded = base64_encode(data, data_size);

    EXPECT_EQ("ZHNsaW5rLWNwcA==", encoded);

    EXPECT_EQ(0, strncmp((const char*)data, base64_decode(encoded).c_str(), data_size));
  }
  {
    const uint8_t data[] = "-cpp";
    size_t data_size = sizeof(data)/sizeof(uint8_t);

    std::string encoded = base64_encode(data, data_size);

    EXPECT_EQ("LWNwcA==", encoded);

    EXPECT_EQ(0, strncmp((const char*)data, base64_decode(encoded).c_str(), data_size));
  }
  {
    const uint8_t data[] = "cpp";
    size_t data_size = sizeof(data)/sizeof(uint8_t);

    std::string encoded = base64_encode(data, data_size);

    EXPECT_EQ("Y3Bw", encoded);

    EXPECT_EQ(0, strncmp((const char*)data, base64_decode(encoded).c_str(), data_size));
  }
}
