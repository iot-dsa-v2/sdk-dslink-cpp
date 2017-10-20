#include "dsa/variant.h"
#include "gtest/gtest.h"

using namespace dsa;

TEST(VariantTest, BaseTest) {
  {
    Var v;
    EXPECT_TRUE(v.is_null());
  }
  {
    Var v("hello");
    EXPECT_TRUE(v.is_string());
  }
  {
    std::string s("hello");
    Var v(std::move(s));
    EXPECT_TRUE(v.is_string());
  }
  {
    Var v(123);
    EXPECT_TRUE(v.is_int());
    EXPECT_EQ(v.get_int(), 123);
  }
  {
    Var v(1.23);
    EXPECT_TRUE(v.is_double());
    EXPECT_EQ(v.get_double(), 1.23);
  }
  {
    Var v(true);
    EXPECT_TRUE(v.is_bool());
    EXPECT_EQ(v.get_bool(), true);
  }
  {
    std::vector<uint8_t> vec = {0, 1, 2};
    Var v(std::move(vec));
    EXPECT_TRUE(v.is_binary());
  }
}

TEST(VariantTest, MapTest) {
  Var v = Var::new_map();
  EXPECT_TRUE(v.is_map());
}

TEST(VariantTest, ArrayTest) {
  Var v = Var::new_array();
  EXPECT_TRUE(v.is_array());
}

TEST(VariantTest, BinaryTest) {
  Var v;
  {
    std::vector<uint8_t> vec = {0, 1, 2};
    Var v0(vec);
    EXPECT_TRUE(v0.is_binary());
    v = v0;
  }
  EXPECT_TRUE(v.is_binary());

  const std::vector<uint8_t>& vec = v.get_binary();
  EXPECT_EQ(vec[1], 1);
}

TEST(VariantTest, InitializerList__Array) {
  Var v{Var("hello"), Var(123), Var(true)};

  EXPECT_TRUE(v.is_array());

  VarArray& vec = v.get_array();

  EXPECT_EQ(3, vec.size());

  EXPECT_TRUE(vec[0].is_string());
  EXPECT_EQ("hello", vec[0].get_string());

  EXPECT_TRUE(vec[1].is_int());
  EXPECT_EQ(123, vec[1].get_int());

  EXPECT_TRUE(vec[2].is_bool());
  EXPECT_TRUE(vec[2].get_bool());
}

TEST(VariantTest, InitilizerList__Map) {
  Var v{{"string", Var("hello")},
            {"int", Var(123)},
            {"bool", Var(true)}};

  EXPECT_TRUE(v.is_map());

  VarMap& map = v.get_map();

  EXPECT_EQ(3, map.size());

  EXPECT_TRUE(map["string"].is_string());
  EXPECT_EQ("hello", map["string"].get_string());

  EXPECT_TRUE(map["int"].is_int());
  EXPECT_EQ(123, map["int"].get_int());

  EXPECT_TRUE(map["bool"].is_bool());
  EXPECT_TRUE(map["bool"].get_bool());
}

TEST(VariantTest, InitilizerList__NestedMap) {
  Var v{{"string", Var("hello")},
            {"int", Var(123)},
            {"map", {{"first", Var("one")}, {"second", Var("two")}}}};

  EXPECT_TRUE(v.is_map());

  VarMap& map = v.get_map();

  EXPECT_EQ(3, map.size());

  EXPECT_TRUE(map["string"].is_string());
  EXPECT_EQ("hello", map["string"].get_string());

  EXPECT_TRUE(map["int"].is_int());
  EXPECT_EQ(123, map["int"].get_int());

  EXPECT_TRUE(map["map"].is_map());

  VarMap& nested_map = map["map"].get_map();

  EXPECT_EQ(2, nested_map.size());

  EXPECT_TRUE(nested_map["first"].is_string());
  EXPECT_EQ("one", nested_map["first"].get_string());

  EXPECT_TRUE(nested_map["second"].is_string());
  EXPECT_EQ("two", nested_map["second"].get_string());
}

TEST(VariantTest, copy) {
  {
    Var v{{"string", Var("hello")},
              {"int", Var(123)},
              {"map", {{"first", Var("one")}, {"second", Var("two")}}}};

    Var v1 = v.copy();

    EXPECT_TRUE(v1.is_map());
    EXPECT_EQ(123, v1.get_map()["int"].get_int());
  }
  {
    Var v{Var("hello"), Var(123), Var(true)};
    Var v1 = v.copy();

    EXPECT_TRUE(v1.is_array());

    VarArray& vec = v1.get_array();

     EXPECT_EQ(3, vec.size());

    EXPECT_TRUE(vec[0].is_string());
    EXPECT_EQ("hello", vec[0].get_string());
  }
}

TEST(VariantTest, deep_copy) {
  // TODO: 'deep test' for deep_copy
  {
    Var v{{"string", Var("hello")},
              {"int", Var(123)},
              {"map", {{"first", Var("one")}, {"second", Var("two")}}}};

    Var v1 = v.deep_copy();

    EXPECT_TRUE(v1.is_map());
    EXPECT_EQ(123, v1.get_map()["int"].get_int());
  }
  {
    Var v{Var("hello"), Var(123), Var(true)};
    Var v1 = v.deep_copy();

    EXPECT_TRUE(v1.is_array());

    VarArray& vec = v1.get_array();

     EXPECT_EQ(3, vec.size());

    EXPECT_TRUE(vec[0].is_string());
    EXPECT_EQ("hello", vec[0].get_string());
  }
}
