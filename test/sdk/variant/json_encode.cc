#include "dsa/variant.h"
#include <gtest/gtest.h>

using namespace dsa;

TEST(VariantTest, JsonEncodingInt) {
  Var v(123);

  string_ encoded_value = v.to_json();

  EXPECT_EQ("123", encoded_value);
}

TEST(VariantTest, JsonEncodingDouble) {
  Var v(1.23);

  string_ encoded_value = v.to_json();

  EXPECT_EQ("1.23", encoded_value);
}

TEST(VariantTest, JsonEncodingBool) {
  {
    Var v(true);

    string_ encoded_value = v.to_json();

    EXPECT_EQ("true", encoded_value);
  }
  {
    Var v(false);

    string_ encoded_value = v.to_json();

    EXPECT_EQ("false", encoded_value);
  }
}

TEST(VariantTest, JsonEncodingString) {
  Var v("hello");

  string_ encoded_value = v.to_json();

  EXPECT_EQ("\"hello\"", encoded_value);
}

TEST(VariantTest, JsonEncodingNull) {
  Var v;

  string_ encoded_value = v.to_json();

  EXPECT_EQ("null", encoded_value);
}

TEST(VariantTest, JsonEncodingArray) {
  Var v{Var("hello"), Var(123), Var(true)};

  string_ encoded_value = v.to_json();

  EXPECT_EQ("[\"hello\",123,true]", encoded_value);
}

TEST(VariantTest, JsonEncodingMap) {
  Var v{{"string", Var("hello")},
            {"int", Var(123)},
            {"bool", Var(true)}};

  string_ encoded_value = v.to_json();

  EXPECT_EQ("{\"bool\":true,\"int\":123,\"string\":\"hello\"}", encoded_value);
}

TEST(VariantTest, JsonEncodingNestedArray) {
  Var v{{"string", Var("hello")},
            {"int", Var(123)},
            {"map", {{"first", Var("one")}, {"second", Var("two")}}}};

  string_ encoded_value = v.to_json();

  EXPECT_EQ("{\"int\":123,\"map\":{\"first\":\"one\",\"second\":\"two\"},\"string\":\"hello\"}", encoded_value);
}


