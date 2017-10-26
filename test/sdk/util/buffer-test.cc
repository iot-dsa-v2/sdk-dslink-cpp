#include "dsa/util.h"
#include "gtest/gtest.h"

using namespace dsa;

TEST(BufferTest, output) {

  std::vector<uint8_t> v = {0x01, 0x02, 0x03, 0x04};
  RefCountBytes ib(v);

  std::stringstream ss;
  ss << ib;

  EXPECT_EQ("[1, 2, 3, 4]", ss.str());
}

