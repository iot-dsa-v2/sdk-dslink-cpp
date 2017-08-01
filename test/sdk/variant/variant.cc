#include "dsa/variant.h"
#include "gtest/gtest.h"

using namespace dsa;
typedef std::unique_ptr<Variant> VariantPtr;

TEST(VariantTest, BaseTest) {
{
    Variant v;
    EXPECT_TRUE(v.is_null());
  }
  {
    VariantPtr v = VariantPtr(Variant::create("hello"));
    EXPECT_TRUE(v->is_string());
  }
  {
    VariantPtr v = VariantPtr(Variant::create(123));
    EXPECT_TRUE(v->is_int());
    EXPECT_EQ(v->get_int(), 123);
  }
  {
    VariantPtr v = VariantPtr(Variant::create(1.23));
    EXPECT_TRUE(v->is_double());
    EXPECT_EQ(v->get_double(), 1.23);
  }
  {
    VariantPtr v = VariantPtr(Variant::create(true));
    EXPECT_TRUE(v->is_bool());
    EXPECT_EQ(v->get_bool(), true);
  }
}

TEST(VariantTest, MapTest) {
  VariantPtr v = VariantPtr(Variant::new_map());
  EXPECT_TRUE(v->is_map());
}

TEST(VariantTest, ArrayTest) {
  VariantPtr v = VariantPtr(Variant::new_array());
  EXPECT_TRUE(v->is_array());
}

TEST(VariantTest, BinaryTest) {
  Variant v;
  {
    std::vector<uint8_t> vec = {0, 1, 2};
    VariantPtr v0 = VariantPtr(Variant::create(vec));
    EXPECT_TRUE(v0->is_binary());
    v = *v0;
  }
  EXPECT_TRUE(v.is_binary());

  const std::vector<uint8_t>& vec = v.get_binary();
  EXPECT_EQ(vec[1], 1);
}
