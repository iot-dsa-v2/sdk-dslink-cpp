#include "dsa/util.h"
#include <gtest/gtest.h>

using namespace dsa;

TEST(DateTimeTest, get) {
  string_ str0 = DateTime::get_ts();

  size_t count = 0;
  while (DateTime::get_ts() == str0) {
    ++count;
  }
  string_ str1 = DateTime::get_ts();
  while (DateTime::get_ts() == str1) {
    ++count;
  }
  string_ str2 = DateTime::get_ts();

  // time should increase
  EXPECT_GT(str1, str0);
  EXPECT_GT(str2, str1);

  // should be able to run multiple times per millisecond
  // EXPECT_GT(count, 0) << str0 << " " << str1 << " " << str2;
}


TEST(DateTimeTest, parse) {
  EXPECT_EQ(DateTime::parse_ts("1970-01-01T00:00:00.000Z"), 0);
  EXPECT_EQ(DateTime::parse_ts("1970-01-01T00:00:00.000+00:00"), 0);
  EXPECT_EQ(DateTime::parse_ts("1970-01-01T00:00:00.001+00:00"), 1);
  EXPECT_EQ(DateTime::parse_ts("1969-12-31T23:59:59.999+00:00"), -1);
  EXPECT_EQ(DateTime::parse_ts("1970-01-01T00:00:00.000+08:00"), -8*3600*1000);
  EXPECT_EQ(DateTime::parse_ts("1970-01-01T00:00:00.000-07:00"), 7*3600*1000);

  EXPECT_EQ(DateTime::parse_ts("2018-04-17T15:43:49.928-07:00"), 1524005029928);
  EXPECT_EQ(DateTime::parse_ts("2018-04-17T15:43:49.928000000-07:00"), 1524005029928);

  EXPECT_EQ(DateTime::parse_ts("2018-04-17T15:43:49-07:00"), 1524005029000);
  EXPECT_EQ(DateTime::parse_ts("2018-04-17T15:43-07:00"), 1524004980000);
}


