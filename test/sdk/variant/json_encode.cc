#include "dsa/variant.h"
#include "gtest/gtest.h"

using namespace dsa;

TEST(VariantTest, JsonEncodingInt) {
  Variant v(123);

  std::string encoded_value = v.to_json();

  EXPECT_EQ("{\"\": 123}", encoded_value);
}

TEST(VariantTest, JsonEncodingDouble) {
  Variant v(1.23);

  std::string encoded_value = v.to_json();

  EXPECT_EQ("{\"\": 1.23}", encoded_value);
}

TEST(VariantTest, JsonEncodingBool) {
  {
    Variant v(true);

    std::string encoded_value = v.to_json();

    EXPECT_EQ("{\"\": true}", encoded_value);
  }
  {
    Variant v(false);

    std::string encoded_value = v.to_json();

    EXPECT_EQ("{\"\": false}", encoded_value);
  }
}

TEST(VariantTest, JsonEncodingString) {
  Variant v("hello");

  std::string encoded_value = v.to_json();

  EXPECT_EQ("{\"\": \"hello\"}", encoded_value);
}

TEST(VariantTest, JsonEncodingNull) {
  Variant v;

  std::string encoded_value = v.to_json();

  EXPECT_EQ("{\"\": null}", encoded_value);
}

TEST(VariantTest, JsonEncodingArray) {
  Variant v{Variant("hello"), Variant(123), Variant(true)};

  std::string encoded_value = v.to_json();

  EXPECT_EQ("{\"\": [\"hello\", 123, true]}", encoded_value);
}

TEST(VariantTest, JsonEncodingMap) {
  Variant v{{"string", Variant("hello")},
            {"int", Variant(123)},
            {"bool", Variant(true)}};

  std::string encoded_value = v.to_json();

  EXPECT_EQ("{\"\": {\"bool\": true, \"int\": 123, \"string\": \"hello\"}}", encoded_value);
}

TEST(VariantTest, JsonEncodingNestedArray) {
  Variant v{{"string", Variant("hello")},
            {"int", Variant(123)},
            {"map", {{"first", Variant("one")}, {"second", Variant("two")}}}};

  std::string encoded_value = v.to_json();

  EXPECT_EQ("{\"\": {\"int\": 123, \"map\": {\"first\": \"one\", \"second\": \"two\"}, \"string\": \"hello\"}}", encoded_value);
}


