#include "dsa/util.h"
#include "gtest/gtest.h"

TEST(DateTime, DateTime) {
  std::string str0 = dsa::DateTime::get_ts();

  size_t count = 0;
  while (dsa::DateTime::get_ts() == str0) {
    ++count;
  }
  std::string str1 = dsa::DateTime::get_ts();
  while (dsa::DateTime::get_ts() == str1) {
    ++count;
  }
  std::string str2 = dsa::DateTime::get_ts();

  // time should increase
  EXPECT_GT(str1, str0);
  EXPECT_GT(str2, str1);

  // should be able to run multiple times per millisecond
  EXPECT_GT(count, 0);
}
