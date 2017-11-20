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
    string_ s("hello");
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
  {
    Var v = Var("12345678901234567890123456789012"
                "12345678901234567890123456789012"
                "12345678901234567890123456789012"
                "12345678901234567890123456789012"
                "12345678901234567890123456789012"
                "12345678901234567890123456789012"
                "12345678901234567890123456789012"
                "12345678901234567890123456789012"
                "12345678901234567890123456789012"
                "12345678901234567890123456789012"
  		);

    Var vdup = v.deep_copy();

    EXPECT_EQ(vdup.get_string(), v.get_string());
    EXPECT_NE(boost::get<ref_<const RefCountString>>(vdup).get(),
              boost::get<ref_<const RefCountString>>(v).get());
  }
  {
    std::vector<uint8_t> x = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                               0, 1, 2, 3, 4, 5, 6, 7, 8, 9
                             };

    Var v(x);
    Var vdup = v.deep_copy();

    EXPECT_EQ(vdup.get_binary(), v.get_binary());
    EXPECT_NE(boost::get<ref_<const RefCountBytes>>(vdup).get(),
              boost::get<ref_<const RefCountBytes>>(v).get());
  }
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



TEST(VariantTest, equality) {
  // Blank
  Var v_blank;
  EXPECT_TRUE(v_blank == v_blank);

  //String
  auto v_string_1_a = Var("hello1");
  EXPECT_FALSE(v_string_1_a == v_blank);

  auto v_string_1_b = Var("hello1");
  auto v_string_2_a = Var("h");

  EXPECT_TRUE(v_string_1_a == v_string_1_a);
  EXPECT_TRUE(v_string_1_a == v_string_1_b);
  EXPECT_FALSE(v_string_1_a == v_string_2_a);

  //Int
  Var v_int_1_a(int(99));
  EXPECT_FALSE(v_int_1_a == v_blank);
  EXPECT_FALSE(v_int_1_a == v_string_1_a);

  Var v_int_1_b(int(99));
  Var v_int_1_c(int(1001));

  EXPECT_TRUE(v_int_1_a == v_int_1_a);
  EXPECT_TRUE(v_int_1_a == v_int_1_b);
  EXPECT_FALSE(v_int_1_a == v_int_1_c);

  // Double
  Var v_double_1_a(1.23);
  EXPECT_FALSE(v_double_1_a == v_blank);
  EXPECT_FALSE(v_double_1_a == v_string_1_a);
  EXPECT_FALSE(v_double_1_a == v_int_1_a);

  Var v_double_1_b(1.23);
  Var v_double_2_a(2.23);
  EXPECT_TRUE(v_double_1_a == v_double_1_a);
  EXPECT_TRUE(v_double_1_a == v_double_1_b);
  EXPECT_FALSE(v_double_1_a == v_double_2_a);

  // Bool
  Var v_bool_1_a(true);
  EXPECT_FALSE(v_bool_1_a == v_blank);
  EXPECT_FALSE(v_bool_1_a == v_string_1_a);
  EXPECT_FALSE(v_bool_1_a == v_int_1_a);
  EXPECT_FALSE(v_bool_1_a == v_double_1_a);

  Var v_bool_1_b(true);
  Var v_bool_2_a(false);
  EXPECT_TRUE(v_bool_1_a == v_bool_1_a);
  EXPECT_TRUE(v_bool_1_a == v_bool_1_b);
  EXPECT_FALSE(v_bool_1_a == v_bool_2_a);

  // Binary
  std::vector<uint8_t> vec = {0, 1, 2, 3};
  Var v_vector_1_a(vec);
  EXPECT_FALSE(v_vector_1_a == v_blank);
  EXPECT_FALSE(v_vector_1_a == v_string_1_a);
  EXPECT_FALSE(v_vector_1_a == v_int_1_a);
  EXPECT_FALSE(v_vector_1_a == v_double_1_a);
  EXPECT_FALSE(v_vector_1_a == v_bool_1_a);

  Var v_vector_1_b(vec);
  vec.push_back(4);
  Var v_vector_2_a(vec);
  EXPECT_TRUE(v_vector_1_a == v_vector_1_a);
  EXPECT_TRUE(v_vector_1_a == v_vector_1_b);
  EXPECT_FALSE(v_vector_1_a == v_vector_2_a);


  //Shared Binary
  std::vector<uint8_t> vec_big{0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
                               0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
                               0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
                               0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
                               0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
                               0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
                               0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
                               0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
                               0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
                               0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
                               0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9};
  Var v_shared_bin_1_a(vec_big);
  EXPECT_FALSE(v_shared_bin_1_a == v_blank);
  EXPECT_FALSE(v_shared_bin_1_a == v_string_1_a);
  EXPECT_FALSE(v_shared_bin_1_a == v_int_1_a);
  EXPECT_FALSE(v_shared_bin_1_a == v_double_1_a);
  EXPECT_FALSE(v_shared_bin_1_a == v_bool_1_a);
  EXPECT_FALSE(v_shared_bin_1_a == v_vector_1_a);
  Var v_shared_bin_1_b(vec_big);
  vec_big.push_back(10);
  Var v_shared_bin_2_a(vec_big);
  EXPECT_TRUE(v_shared_bin_1_a == v_shared_bin_1_a);
  EXPECT_TRUE(v_shared_bin_1_a == v_shared_bin_1_b);
  EXPECT_FALSE(v_shared_bin_1_a == v_shared_bin_2_a);

  //Shared String
  std::string big = "abcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefg"
          "abcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefg"
          "abcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefg"
          "abcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefg"
          "abcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefgabcdefg";
  Var v_shared_str_1_a(big);
  EXPECT_FALSE(v_shared_str_1_a == v_blank);
  EXPECT_FALSE(v_shared_str_1_a == v_string_1_a);
  EXPECT_FALSE(v_shared_str_1_a == v_int_1_a);
  EXPECT_FALSE(v_shared_str_1_a == v_double_1_a);
  EXPECT_FALSE(v_shared_str_1_a == v_bool_1_a);
  EXPECT_FALSE(v_shared_str_1_a == v_vector_1_a);
  EXPECT_FALSE(v_shared_str_1_a == v_shared_bin_1_a);
  Var v_shared_str_1_b(big);
  big[0] = '1';
  Var v_shared_str_2_a(big);
  EXPECT_TRUE(v_shared_str_1_a == v_shared_str_1_a);
  EXPECT_TRUE(v_shared_str_1_a == v_shared_str_1_b);
  EXPECT_FALSE(v_shared_str_1_a == v_shared_str_2_a);

  // DIFFERENT TYPES Successful Equality
  // Vector - Str
  std::vector<uint8_t> vec_str = {'h', 'e', 'l', 'l', 'o'};
  Var v_str_c("hello");
  Var v_vec_c(std::move(vec_str));
  EXPECT_TRUE(v_str_c == v_vec_c);

  // Shared Binary - Shared Str
  std::string shared_str;
  std::vector<uint8_t> shared_bin;
  for(int i = 0; i < 1024; i++)
  {
    shared_str.push_back(i % 77);
    shared_bin.push_back(i % 77);
  }

  EXPECT_TRUE(Var(shared_str) == Var(shared_bin));
}