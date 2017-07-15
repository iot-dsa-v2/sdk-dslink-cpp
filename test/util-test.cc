//
// Created by Ben Richards on 7/13/17.
//
#include <iostream>

#include "dsa/util.h"
#include "gtest/gtest.h"

/**** class Buffer ****/
TEST(BufferTest, WillPrint) {
  dsa::Buffer buf(50);
  for (size_t i = 0; i < 50; ++i) buf[i] = uint8_t(i);
  std::cout << buf << std::endl;
}

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

TEST(BufferTest, ValidResize) {
  dsa::Buffer buf(10);
  EXPECT_EQ(buf.size(), 0);
  EXPECT_EQ(buf.capacity(), 10);
  buf.resize(15);
  EXPECT_EQ(buf.size(), 0);
  EXPECT_EQ(buf.capacity(), 15);
}

class RefCheck : public dsa::EnableShared<RefCheck> {
 public:
  static int count;
  RefCheck(int a) { ++count; }
  ~RefCheck() { --count; }
};
int RefCheck::count = 0;

TEST(EnableSharedTest, RefCheck) {
  std::shared_ptr<RefCheck> ptr = dsa::make_shared<RefCheck>(1);
  EXPECT_EQ(RefCheck::count, 1);
  ptr->destory();
  EXPECT_EQ(RefCheck::count, 1);
  ptr.reset();
  EXPECT_EQ(RefCheck::count, 0);
}
