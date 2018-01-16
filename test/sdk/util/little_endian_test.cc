#include "dsa/util.h"
#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

TEST(LittleEndianTest, Write) {
  std::vector<uint8_t> data = {1, 2, 3, 4, 5, 6, 7};

  std::vector<uint8_t> expect16 = {1, 0x22, 0x11, 4, 5, 6, 7};
  std::vector<uint8_t> expect32 = {1, 0x66, 0x55, 0x44, 0x33, 6, 7};

  uint16_t nn = 0x1122;
  uint32_t nnnn = 0x33445566;

  dsa::write_16_t(&data[1], nn);
  EXPECT_TRUE(std::equal(data.begin(), data.end(), expect16.begin()));

  dsa::write_32_t(&data[1], nnnn);
  EXPECT_TRUE(std::equal(data.begin(), data.end(), expect32.begin()));
}

TEST(LittleEndianTest, Read) {
  std::vector<uint8_t> data = {1, 0x22, 0x11, 0x66, 0x55, 0x44, 0x33};

  EXPECT_EQ(0x1122, dsa::read_16_t(&data[1]));
  EXPECT_EQ(0x33445566, dsa::read_32_t(&data[3]));
}
