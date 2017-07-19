//
// Created by Ben Richards on 7/13/17.
//
#include <iostream>

#include "dsa/util.h"
#include "gtest/gtest.h"

/**** class Buffer ****/
TEST(BufferTest, SafeAppendResize) {
  dsa::Buffer buf(10);
  for (size_t i = 0; i < 10; ++i) {
    buf.safe_append('A');
    EXPECT_EQ(i + 1, buf.size());
    EXPECT_EQ(10, buf.capacity());
  }
  buf.safe_append('A');
  EXPECT_EQ(20, buf.capacity());
}

TEST(BufferTest, Resize) {
  dsa::Buffer buf(10);
  EXPECT_EQ(buf.size(), 0);
  EXPECT_EQ(buf.capacity(), 10);
  buf.resize(15);
  EXPECT_EQ(buf.size(), 0);
  EXPECT_EQ(buf.capacity(), 15);
  buf.resize(10);
  EXPECT_EQ(buf.size(), 0);
  EXPECT_EQ(buf.capacity(), 15);
  buf[9] = 5;
  EXPECT_EQ(buf.size(), 10);
  EXPECT_EQ(buf.capacity(), 15);
  ASSERT_ANY_THROW(buf[1000000]);
}
