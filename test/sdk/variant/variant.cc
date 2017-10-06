#include "dsa/variant.h"
#include "gtest/gtest.h"

using namespace dsa;

TEST(VariantTest, BaseTest) {
  {
    Variant v;
    EXPECT_TRUE(v.is_null());
  }
  {
    Variant v("hello");
    EXPECT_TRUE(v.is_string());
  }
  {
    std::string s("hello");
    Variant v(std::move(s));
    EXPECT_TRUE(v.is_string());
  }
  {
    Variant v(123);
    EXPECT_TRUE(v.is_int());
    EXPECT_EQ(v.get_int(), 123);
  }
  {
    Variant v(1.23);
    EXPECT_TRUE(v.is_double());
    EXPECT_EQ(v.get_double(), 1.23);
  }
  {
    Variant v(true);
    EXPECT_TRUE(v.is_bool());
    EXPECT_EQ(v.get_bool(), true);
  }
  {
    std::vector<uint8_t> vec = {0, 1, 2};
    Variant v(std::move(vec));
    EXPECT_TRUE(v.is_binary());
  }
}

TEST(VariantTest, MapTest) {
  Variant v = Variant::new_map();
  EXPECT_TRUE(v.is_map());
}

TEST(VariantTest, ArrayTest) {
  Variant v = Variant::new_array();
  EXPECT_TRUE(v.is_array());
}

TEST(VariantTest, BinaryTest) {
  Variant v;
  {
    std::vector<uint8_t> vec = {0, 1, 2};
    Variant v0(vec);
    EXPECT_TRUE(v0.is_binary());
    v = v0;
  }
  EXPECT_TRUE(v.is_binary());

  const std::vector<uint8_t>& vec = v.get_binary();
  EXPECT_EQ(vec[1], 1);
}

TEST(VariantTest, InitializerList__Array) {
  Variant v{Variant("hello"), Variant(123), Variant(true)};

  EXPECT_TRUE(v.is_array());

  VariantArray& vec = v.get_array();

  EXPECT_EQ(3, vec.size());

  EXPECT_TRUE(vec[0].is_string());
  EXPECT_EQ("hello", vec[0].get_string());

  EXPECT_TRUE(vec[1].is_int());
  EXPECT_EQ(123, vec[1].get_int());

  EXPECT_TRUE(vec[2].is_bool());
  EXPECT_TRUE(vec[2].get_bool());
}

TEST(VariantTest, InitilizerList__Map) {
  Variant v{{"string", Variant("hello")},
            {"int", Variant(123)},
            {"bool", Variant(true)}};

  EXPECT_TRUE(v.is_map());

  VariantMap& map = v.get_map();

  EXPECT_EQ(3, map.size());

  EXPECT_TRUE(map["string"].is_string());
  EXPECT_EQ("hello", map["string"].get_string());

  EXPECT_TRUE(map["int"].is_int());
  EXPECT_EQ(123, map["int"].get_int());

  EXPECT_TRUE(map["bool"].is_bool());
  EXPECT_TRUE(map["bool"].get_bool());
}

TEST(VariantTest, InitilizerList__NestedMap) {
  Variant v{{"string", Variant("hello")},
            {"int", Variant(123)},
            {"map", {{"first", Variant("one")}, {"second", Variant("two")}}}};

  EXPECT_TRUE(v.is_map());

  VariantMap& map = v.get_map();

  EXPECT_EQ(3, map.size());

  EXPECT_TRUE(map["string"].is_string());
  EXPECT_EQ("hello", map["string"].get_string());

  EXPECT_TRUE(map["int"].is_int());
  EXPECT_EQ(123, map["int"].get_int());

  EXPECT_TRUE(map["map"].is_map());

  VariantMap& nested_map = map["map"].get_map();

  EXPECT_EQ(2, nested_map.size());

  EXPECT_TRUE(nested_map["first"].is_string());
  EXPECT_EQ("one", nested_map["first"].get_string());

  EXPECT_TRUE(nested_map["second"].is_string());
  EXPECT_EQ("two", nested_map["second"].get_string());
}

TEST(VariantTest, copy) {
  {
    Variant v{{"string", Variant("hello")},
              {"int", Variant(123)},
              {"map", {{"first", Variant("one")}, {"second", Variant("two")}}}};

    Variant v1 = v.copy();

    EXPECT_TRUE(v1.is_map());
    EXPECT_EQ(123, v1.get_map()["int"].get_int());
  }
  {
    Variant v{Variant("hello"), Variant(123), Variant(true)};
    Variant v1 = v.copy();

    EXPECT_TRUE(v1.is_array());

    VariantArray& vec = v1.get_array();

     EXPECT_EQ(3, vec.size());

    EXPECT_TRUE(vec[0].is_string());
    EXPECT_EQ("hello", vec[0].get_string());
  }
}

TEST(VariantTest, deep_copy) {
  // TODO: 'deep test' for deep_copy
  {
    Variant v{{"string", Variant("hello")},
              {"int", Variant(123)},
              {"map", {{"first", Variant("one")}, {"second", Variant("two")}}}};

    Variant v1 = v.deep_copy();

    EXPECT_TRUE(v1.is_map());
    EXPECT_EQ(123, v1.get_map()["int"].get_int());
  }
  {
    Variant v{Variant("hello"), Variant(123), Variant(true)};
    Variant v1 = v.deep_copy();

    EXPECT_TRUE(v1.is_array());

    VariantArray& vec = v1.get_array();

     EXPECT_EQ(3, vec.size());

    EXPECT_TRUE(vec[0].is_string());
    EXPECT_EQ("hello", vec[0].get_string());
  }
}
