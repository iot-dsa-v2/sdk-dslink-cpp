#include "dsa/variant.h"
#include "gtest/gtest.h"

using namespace dsa;

TEST(VariantTest, JsonDecodingInt) {
  string_ json_string("{\"\":123}");

  Var v = Var::from_json(json_string.c_str());

  string_ encoded_value = v.to_json();

  EXPECT_EQ(json_string, encoded_value);
}

TEST(VariantTest, JsonDecodingDouble) {
  string_ json_string("{\"\":1.23}");

  Var v = Var::from_json(json_string.c_str());

  string_ encoded_value = v.to_json();

  EXPECT_EQ(json_string, encoded_value);
}

TEST(VariantTest, JsonDecodingBool) {
  {
    string_ json_string("{\"\":true}");

    Var v = Var::from_json(json_string.c_str());

    string_ encoded_value = v.to_json();

    EXPECT_EQ(json_string, encoded_value);
  }
  {
    string_ json_string("{\"\":false}");

    Var v = Var::from_json(json_string.c_str());

    string_ encoded_value = v.to_json();

    EXPECT_EQ(json_string, encoded_value);
  }
}

TEST(VariantTest, JsonDecodingString) {
  string_ json_string("{\"\":\"hello\"}");

  Var v = Var::from_json(json_string.c_str());

  string_ encoded_value = v.to_json();

  EXPECT_EQ(json_string, encoded_value);
}

TEST(VariantTest, JsonDecodingNull) {
  string_ json_string("{\"\":null}");

  Var v = Var::from_json(json_string.c_str());

  string_ encoded_value = v.to_json();

  EXPECT_EQ(json_string, encoded_value);
}

TEST(VariantTest, JsonDecodingArray) {
  string_ json_string("{\"\":[\"hello\",123,true]}");

  Var v = Var::from_json(json_string.c_str());

  string_ encoded_value = v.to_json();

  EXPECT_EQ(json_string, encoded_value);
}

TEST(VariantTest, JsonDecodingMap) {
  string_ json_string("{\"\":{\"bool\":true,\"int\":123,\"string\":\"hello\"}}");

  Var v = Var::from_json(json_string.c_str());

  string_ encoded_value = v.to_json();

  EXPECT_EQ(json_string, encoded_value);
}

TEST(VariantTest, JsonDecodingNestedArray) {
  string_ json_string("{\"\":{\"int\":123,\"map\":{\"first\":\"one\",\"second\":\"two\"},\"string\":\"hello\"}}");

  Var v = Var::from_json(json_string.c_str());

  string_ encoded_value = v.to_json();

  EXPECT_EQ(json_string, encoded_value);
}






