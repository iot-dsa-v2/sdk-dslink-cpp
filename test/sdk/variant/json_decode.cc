#include "dsa/variant.h"
#include "gtest/gtest.h"

using namespace dsa;

TEST(VariantTest, JsonDecodingInt) {
  std::string json_string("{\"\": 123}");

  Var v = Var::from_json(json_string.c_str());

  std::string encoded_value = v.to_json();

  EXPECT_EQ(json_string, encoded_value);
}

TEST(VariantTest, JsonDecodingDouble) {
  std::string json_string("{\"\": 1.23}");

  Var v = Var::from_json(json_string.c_str());

  std::string encoded_value = v.to_json();

  EXPECT_EQ(json_string, encoded_value);
}

TEST(VariantTest, JsonDecodingBool) {
  {
    std::string json_string("{\"\": true}");

    Var v = Var::from_json(json_string.c_str());

    std::string encoded_value = v.to_json();

    EXPECT_EQ(json_string, encoded_value);
  }
  {
    std::string json_string("{\"\": false}");

    Var v = Var::from_json(json_string.c_str());

    std::string encoded_value = v.to_json();

    EXPECT_EQ(json_string, encoded_value);
  }
}

TEST(VariantTest, JsonDecodingString) {
  std::string json_string("{\"\": \"hello\"}");

  Var v = Var::from_json(json_string.c_str());

  std::string encoded_value = v.to_json();

  EXPECT_EQ(json_string, encoded_value);
}

TEST(VariantTest, JsonDecodingNull) {
  std::string json_string("{\"\": null}");

  Var v = Var::from_json(json_string.c_str());

  std::string encoded_value = v.to_json();

  EXPECT_EQ(json_string, encoded_value);
}

TEST(VariantTest, JsonDecodingArray) {
  std::string json_string("{\"\": [\"hello\", 123, true]}");

  Var v = Var::from_json(json_string.c_str());

  std::string encoded_value = v.to_json();

  EXPECT_EQ(json_string, encoded_value);
}

TEST(VariantTest, JsonDecodingMap) {
  std::string json_string("{\"\": {\"bool\": true, \"int\": 123, \"string\": \"hello\"}}");

  Var v = Var::from_json(json_string.c_str());

  std::string encoded_value = v.to_json();

  EXPECT_EQ(json_string, encoded_value);
}

TEST(VariantTest, JsonDecodingNestedArray) {
  std::string json_string("{\"\": {\"int\": 123, \"map\": {\"first\": \"one\", \"second\": \"two\"}, \"string\": \"hello\"}}");

  Var v = Var::from_json(json_string.c_str());

  std::string encoded_value = v.to_json();

  EXPECT_EQ(json_string, encoded_value);
}






