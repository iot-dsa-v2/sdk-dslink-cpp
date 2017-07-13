//
// Created by Ben Richards on 7/13/17.
//
#include <iostream>

#include "../cmake-build-release/test/lib/googletest-src/googletest/include/gtest/gtest.h"
#include "dsa/util.h"

TEST(BufferTest, TestPrint) {
  dsa::Buffer buf(100);
  for (size_t i = 0; i < 100; ++i)
    buf[i] = uint8_t(i);
  std::cout << buf << std::endl;
}