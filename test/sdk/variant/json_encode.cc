#include "dsa/variant.h"
#include "gtest/gtest.h"

using namespace dsa;

TEST(VariantTest, JsonEncodingInt) {
  Variant v(123);

  std::string encoded_msg = v.to_json();

  EXPECT_EQ("{\"\": 123}", encoded_msg);
}

TEST(VariantTest, JsonEncodingDouble) {
  Variant v(1.23);

  std::string encoded_msg = v.to_json();

  EXPECT_EQ("{\"\": 1.23}", encoded_msg);
}

TEST(VariantTest, JsonEncodingBool) {
  {
    Variant v(true);

    std::string encoded_msg = v.to_json();

    EXPECT_EQ("{\"\": true}", encoded_msg);
  }
  {
    Variant v(false);

    std::string encoded_msg = v.to_json();

    EXPECT_EQ("{\"\": false}", encoded_msg);
  }
}

TEST(VariantTest, JsonEncodingString) {
  Variant v("hello");

  std::string encoded_msg = v.to_json();

  EXPECT_EQ("{\"\": \"hello\"}", encoded_msg);
}


TEST(VariantTest, JsonEncodingNull) {
  Variant v;

  std::string encoded_msg = v.to_json();

  EXPECT_EQ("{\"\": null}", encoded_msg);
}
